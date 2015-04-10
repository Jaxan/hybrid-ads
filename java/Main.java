package yannakakis;

import com.google.common.collect.Lists;
import de.learnlib.acex.analyzers.AcexAnalyzers;
import de.learnlib.algorithms.dhc.mealy.MealyDHC;
import de.learnlib.algorithms.kv.mealy.KearnsVaziraniMealy;
import de.learnlib.algorithms.lstargeneric.ce.ObservationTableCEXHandlers;
import de.learnlib.algorithms.lstargeneric.closing.ClosingStrategies;
import de.learnlib.algorithms.lstargeneric.mealy.ExtensibleLStarMealy;
import de.learnlib.algorithms.ttt.mealy.TTTLearnerMealy;
import de.learnlib.algorithms.ttt.mealy.TTTLearnerMealyBuilder;
import de.learnlib.api.EquivalenceOracle;
import de.learnlib.api.LearningAlgorithm;
import de.learnlib.api.MembershipOracle;
import de.learnlib.api.Query;
import de.learnlib.counterexamples.LocalSuffixFinders;
import de.learnlib.eqtests.basic.EQOracleChain;
import de.learnlib.eqtests.basic.SimulatorEQOracle;
import de.learnlib.eqtests.basic.WMethodEQOracle;
import de.learnlib.eqtests.basic.WpMethodEQOracle;
import de.learnlib.oracles.AbstractSingleQueryOracle;
import de.learnlib.oracles.DefaultQuery;
import de.learnlib.oracles.SimulatorOracle;
import de.learnlib.parallelism.DynamicParallelOracle;
import de.learnlib.parallelism.ParallelOracle;
import de.learnlib.parallelism.ParallelOracleBuilders;
import net.automatalib.automata.transout.MealyMachine;
import net.automatalib.automata.transout.impl.compact.CompactMealy;
import net.automatalib.util.graphs.dot.GraphDOT;
import net.automatalib.words.Alphabet;
import net.automatalib.words.Word;
import net.automatalib.words.WordBuilder;
import net.automatalib.words.impl.Alphabets;

import javax.annotation.Nullable;
import java.io.IOException;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.nio.file.Paths;
import java.util.*;
import java.util.function.Consumer;
import java.util.function.Supplier;

import static de.learnlib.cache.mealy.MealyCaches.createCache;

/**
 * Test for the Lee and Yannakakis implementation.
 */
public class Main {

	public static void main(String[] args) throws IOException {
		// We read the mealy machine we want to simulate
		String filename = "esm-manual-controller.dot";
		System.out.println("Reading dot file: " + filename);
		GraphvizParser p = new GraphvizParser(Paths.get(filename));
		CompactMealy<String, String> fm = p.createMachine();

		Alphabet<String> alphabet = fm.getInputAlphabet();
		Alphabet<String> subAlphabet = Alphabets.fromArray("21.1", "21.0", "22", "53.4", "52.5");
		List<Alphabet<String>> alphabets = Arrays.asList(subAlphabet, alphabet);

		System.out.println("created machine with " + fm.size() + " states and " + alphabet.size() + " inputs\n");


		// We will have the simulating membership oracle. Since the equivalence oracles use a membership oracle to
		// test equality, we make membership oracle which maintain a count. E.g. This way we can count the number of
		// queries needed to find a counterexample.
		System.out.println("Setting up membership oracles");
		SimulatorOracle.MealySimulatorOracle<String, String> mOracleMealy = new SimulatorOracle.MealySimulatorOracle<>(fm);
		DynamicParallelOracle<String, Word<String>> mParallelOracle = ParallelOracleBuilders.newDynamicParallelOracle(() -> {
			return new SimulatorOracle.MealySimulatorOracle<>(fm);
		}).withBatchSize(5)
				.withPoolSize(4)
				.withPoolPolicy(ParallelOracle.PoolPolicy.FIXED)
				.create();

		CountingMembershipOracle<String, Word<String>> mOracleForLearning = new CountingMembershipOracle<>(mParallelOracle, "learning");
		CountingMembershipOracle<String, Word<String>> mOracleForTesting = new CountingMembershipOracle<>(mParallelOracle, "testing");


		// We can test multiple equivalence oracles. They all treat the SUL as a black box.
		System.out.println("Setting up equivalence oracles");
		WpMethodEQOracle.MealyWpMethodEQOracle<String, String> eqOracleWp = new WpMethodEQOracle.MealyWpMethodEQOracle<>(3, mOracleForTesting);
		WMethodEQOracle.MealyWMethodEQOracle<String, String> eqOracleW = new WMethodEQOracle.MealyWMethodEQOracle<>(2, mOracleForTesting);
		EquivalenceOracle.MealyEquivalenceOracle<String, String> eqOracleYannakakis = new YannakakisEQOracle<>(alphabets, mOracleForTesting);

		// The chosen oracle to experiment with.
		EquivalenceOracle<MealyMachine<?, String, ?, String>, String, Word<String>> eqOracle = eqOracleYannakakis;


		// Learnlib comes with different learning algorithms
		System.out.println("Setting up learner(s)");
		ExtensibleLStarMealy<String, String> learnerLStar = new ExtensibleLStarMealy<>(alphabet, mOracleForLearning, Lists.<Word<String>>newArrayList(), ObservationTableCEXHandlers.CLASSIC_LSTAR, ClosingStrategies.CLOSE_SHORTEST);
		TTTLearnerMealy<String, String> learnerTTT = new TTTLearnerMealy<>(alphabet, mOracleForLearning, LocalSuffixFinders.FIND_LINEAR);
		KearnsVaziraniMealy<String, String> learnerKV = new KearnsVaziraniMealy<>(alphabet, mOracleForLearning, false, AcexAnalyzers.BINARY_SEARCH);

		// The chosen learning algorithm
		LearningAlgorithm<MealyMachine<?, String, ?, String>, String, Word<String>> learner = learnerLStar;


		// Here we will perform our experiment. I did not use the Experiment class from LearnLib, as I wanted some
		// more control (for example, I want to reset the counters in the membership oracles). This control flow
		// is suggested by LearnLib (on their wiki).
		System.out.println("Starting experiment\n");
		int stage = 0;
		learner.startLearning();

		while(true) {
//			String dir = "/Users/joshua/Documents/PhD/Machines/Mealy/esms3/";
//			String filenameh = dir + "hyp." + stage + ".obf.dot";
//			PrintWriter output = new PrintWriter(filenameh);
//			GraphDOT.write(learner.getHypothesisModel(), alphabet, output);
//			output.close();

			System.out.println(stage++ + ": " + Calendar.getInstance().getTime());
			System.out.println("Hypothesis: " + learner.getHypothesisModel().getStates().size());
			mOracleForLearning.logAndReset(System.out);
			mOracleForTesting.logAndReset(System.out);
			System.out.println();

			DefaultQuery<String, Word<String>> ce = eqOracle.findCounterExample(learner.getHypothesisModel(), alphabet);
			if(ce == null) break;

			learner.refineHypothesis(ce);
		}

		System.out.println("Done with learning, no counter example found after:");
		mOracleForLearning.logAndReset(System.out);
		mOracleForTesting.logAndReset(System.out);
		System.out.println();

		PrintWriter output = new PrintWriter("last_hypothesis.dot");
		GraphDOT.write(learner.getHypothesisModel(), alphabet, output);
		output.close();
	}

	/**
	 * An membership oracle which maintains a count of the number of queries (and symbols). Usefull for testing
	 * performance of equivalence oracles (which use membership oracles). It simply delegates the queries to
	 * a delegate.
	 * @param <I> Input alphabet
	 * @param <D> Output domain (should be Word<O> for mealy machines)
	 */
	static public class CountingMembershipOracle<I, D> implements MembershipOracle<I, D> {
		private final MembershipOracle<I, D> delegate;
		private final String name;
		private long queries = 0;
		private long symbols = 0;

		public CountingMembershipOracle(MembershipOracle<I, D> delegate, String name){
			this.delegate = delegate;
			this.name = name;
		}

		public void reset(){
			queries = 0;
			symbols = 0;
		}

		public void log(PrintStream output){
			output.println(name + ": queries: " + queries);
			output.println(name + ": symbols: " + symbols);
		}

		public void logAndReset(PrintStream output){
			log(output);
			reset();
		}

		@Override
		public void processQueries(Collection<? extends Query<I, D>> collection) {
			queries += collection.size();
			collection.parallelStream().forEach((Query<I, D> idQuery) -> {
				symbols += idQuery.getInput().size();
			});

			delegate.processQueries(collection);
		}
	}
}
