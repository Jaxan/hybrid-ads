package yannakakis;

import com.google.common.collect.Lists;
import de.learnlib.algorithms.dhc.mealy.MealyDHC;
import de.learnlib.algorithms.lstargeneric.ce.ObservationTableCEXHandlers;
import de.learnlib.algorithms.lstargeneric.closing.ClosingStrategies;
import de.learnlib.algorithms.lstargeneric.mealy.ExtensibleLStarMealy;
import de.learnlib.api.EquivalenceOracle;
import de.learnlib.api.LearningAlgorithm;
import de.learnlib.api.MembershipOracle;
import de.learnlib.eqtests.basic.EQOracleChain;
import de.learnlib.eqtests.basic.SimulatorEQOracle;
import de.learnlib.eqtests.basic.WMethodEQOracle;
import de.learnlib.eqtests.basic.WpMethodEQOracle;
import de.learnlib.oracles.AbstractSingleQueryOracle;
import de.learnlib.oracles.DefaultQuery;
import de.learnlib.oracles.SimulatorOracle;
import net.automatalib.automata.transout.MealyMachine;
import net.automatalib.automata.transout.impl.compact.CompactMealy;
import net.automatalib.util.graphs.dot.GraphDOT;
import net.automatalib.words.Alphabet;
import net.automatalib.words.Word;
import net.automatalib.words.WordBuilder;

import javax.annotation.Nullable;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;

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

		System.out.println("created machine with " + fm.size() + " states and " + alphabet.size() + " inputs\n");


		// We will have the simulating membership oracle. Since the equivalence oracles use a membership oracle to
		// test equality, we make membership oracle which maintain a count. E.g. This way we can count the number of
		// queries needed to find a counterexample.
		System.out.println("Setting up membership oracles");
		SimulatorOracle.MealySimulatorOracle<String, String> mOracleMealy = new SimulatorOracle.MealySimulatorOracle<>(fm);
		CountingQueryOracle<String, Word<String>> mOracleCounting1 = new CountingQueryOracle<>(mOracleMealy);
		CountingQueryOracle<String, Word<String>> mOracleCounting2 = new CountingQueryOracle<>(mOracleMealy);


		// We can test multiple equivalence oracles. The eqOracleMealy sees the SUL as a white box mealy machine to
		// find a counterexample. The others all treat the SUL as a black box. Since there was one specific
		// counterexample which was hard to find, I created an oracle precisely for that example, and chain it with
		// the Lee and Yannakakis oracle.
		System.out.println("Setting up equivalence oracles");
		SimulatorEQOracle.MealySimulatorEQOracle<String, String> eqOracleMealy = new SimulatorEQOracle.MealySimulatorEQOracle<>(fm);
		WpMethodEQOracle.MealyWpMethodEQOracle<String, String> eqOracleWp = new WpMethodEQOracle.MealyWpMethodEQOracle<>(3, mOracleCounting2);
		WMethodEQOracle.MealyWMethodEQOracle<String, String> eqOracleW = new WMethodEQOracle.MealyWMethodEQOracle<>(2, mOracleCounting2);
		EquivalenceOracle.MealyEquivalenceOracle<String, String> eqOracleYannakakis = new YannakakisEQOracle<>(mOracleCounting2);
		EquivalenceOracle.MealyEquivalenceOracle<String, String> eqOracleSpecific = new SpecificCounterExampleOracle(mOracleCounting2);
		EQOracleChain.MealyEQOracleChain eqOracleYannakakisPlus = new EQOracleChain.MealyEQOracleChain(Arrays.asList(eqOracleSpecific, eqOracleYannakakis));

		// The chosen oracle to experiment with.
		EQOracleChain.MealyEQOracleChain eqOracle = eqOracleYannakakisPlus;


		// Learnlib comes with different learning algorithms
		System.out.println("Setting up learner(s)");
		MealyDHC<String, String> learnerDHC = new MealyDHC<>(alphabet, mOracleCounting1);
		ExtensibleLStarMealy<String, String> learnerLStar = new ExtensibleLStarMealy<>(alphabet, mOracleCounting1, Lists.<Word<String>>newArrayList(), ObservationTableCEXHandlers.CLASSIC_LSTAR, ClosingStrategies.CLOSE_FIRST);

		// The chosen learning algorithm
		LearningAlgorithm.MealyLearner<String, String> learner = learnerLStar;


		// Here we will perform our experiment. I did not use the Experiment class from LearnLib, as I wanted some
		// more control (for example, I want to reset the counters in the membership oracles). This control flow
		// is suggested by LearnLib (on their wiki).
		System.out.println("Starting experiment\n");
		int stage = 0;
		learner.startLearning();

		while(true) {
			DefaultQuery<String, Word<String>> ce = eqOracle.findCounterExample(learner.getHypothesisModel(), alphabet);
			if(ce == null) break;

			learner.refineHypothesis(ce);

			// FIXME: Make this a command line option
			String dir = "/Users/joshua/Documents/PhD/Machines/Mealy/esms3/";
			String filenameh = dir + "hyp." + stage + ".obf.dot";
			PrintWriter output = new PrintWriter(filenameh);
			GraphDOT.write(learner.getHypothesisModel(), alphabet, output);
			output.close();

			System.out.println(stage++ + ": " + Calendar.getInstance().getTime());
			System.out.println("Hypothesis has " + learner.getHypothesisModel().getStates().size() + " states");
			mOracleCounting1.log_and_reset("learning");
			mOracleCounting2.log_and_reset("finding a counter example");
			System.out.println();
		}

		System.out.println(stage++ + ": " + Calendar.getInstance().getTime());
		System.out.println("Conclusion has " + learner.getHypothesisModel().getStates().size() + " states");
		mOracleCounting1.log_and_reset("learning");
		mOracleCounting2.log_and_reset("finding a counter example");
		System.out.println("Done!");

		PrintWriter output = new PrintWriter("last_hypothesis.dot");
		GraphDOT.write(learner.getHypothesisModel(), alphabet, output);
		output.close();
	}

	/**
	 * An membership oracle which maintains a count of the number of queries (and symbols). Usefull for testing
	 * performance of equivalence oracles (which use membership oracles).
	 * @param <I> Input alphabet
	 * @param <O> Output alphabet
	 */
	public static class CountingQueryOracle<I, O> extends AbstractSingleQueryOracle<I, O>{
		private final AbstractSingleQueryOracle<I, O> delegate;
		public int query_count = 0;
		public int symbol_count = 0;

		CountingQueryOracle(AbstractSingleQueryOracle<I, O> delegate){
			this.delegate = delegate;
		}

		public void reset(){
			query_count = 0;
			symbol_count = 0;
		}

		public void log_and_reset(String message){
			System.out.println("used " + query_count + " queries and " + symbol_count + " symbols for " + message);
			reset();
		}

		@Override
		public O answerQuery(Word<I> word, Word<I> word1) {
			query_count++;
			symbol_count += word.length() + word1.length();
			return delegate.answerQuery(word, word1);
		}
	}

	/**
	 * An equivalence oracle to test a single sequence, which I needed to learn the Oce test case.
	 */
	public static class SpecificCounterExampleOracle implements EquivalenceOracle.MealyEquivalenceOracle<String, String> {
		private final MembershipOracle<String, Word<String>> sulOracle;
		private boolean fired = false;

		SpecificCounterExampleOracle(MembershipOracle<String, Word<String>> sulOracle){
			this.sulOracle = sulOracle;
		}

		@Nullable
		@Override
		public DefaultQuery<String, Word<String>> findCounterExample(MealyMachine<?, String, ?, String> objects, Collection<? extends String> collection) {
			if(fired) return null;

			WordBuilder<String> wb = new WordBuilder<>();
			wb.append("52.5", "53.4", "21.0", "21.0", "21.0", "21.0", "21.0", "21.0", "21.0", "21.0", "21.0", "37.2", "10", "9.4");

			Word<String> test = wb.toWord();
			DefaultQuery<String, Word<String>> query = new DefaultQuery<>(test);
			sulOracle.processQueries(Collections.singleton(query));

			fired = true;
			return query;
		}
	}
}
