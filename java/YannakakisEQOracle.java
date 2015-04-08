package yannakakis;

import de.learnlib.api.EquivalenceOracle;
import de.learnlib.api.MembershipOracle;
import de.learnlib.oracles.DefaultQuery;
import net.automatalib.automata.transout.MealyMachine;
import net.automatalib.util.graphs.dot.GraphDOT;
import net.automatalib.words.Alphabet;
import net.automatalib.words.Word;
import net.automatalib.words.WordBuilder;

import javax.annotation.Nullable;
import java.io.*;
import java.util.*;

/**
 * Implements the Lee & Yannakakis suffixes by invoking an external program. Because of this indirection to an external
 * program, the findCounterexample method might throw a RuntimeException. Sorry for the hard-coded path to the
 * executable!
 *
 * @param <O> is the output alphabet. (So a query will have type Word<String, Word<O>>.)
 */
public class YannakakisEQOracle<O> implements EquivalenceOracle.MealyEquivalenceOracle<String, O> {
	private final MembershipOracle<String, Word<O>> sulOracle;
	private final List<Alphabet<String>> alphabets;
	private final ProcessBuilder pb;

	int currentAlphabet = 0;
	private long bound = 100;

	private Process process;
	private Writer processInput;
	private BufferedReader processOutput;
	private StreamGobbler errorGobbler;


	/**
	 * @param sulOracle The membership oracle of the SUL, we need this to check the output on the test suite
	 * @throws IOException
	 */
	YannakakisEQOracle(List<Alphabet<String>> alphabets, MembershipOracle<String, Word<O>> sulOracle) throws IOException {
		this.sulOracle = sulOracle;
		this.alphabets = alphabets;
		pb = new ProcessBuilder("/Users/joshua/Documents/PhD/Yannakakis/build/main", "--", "1", "stream");
	}

	/**
	 * A small class to print all stuff to stderr. Useful as I do not want stderr and stdout of the external program to
	 * be merged, but still want to redirect stderr to java's stderr.
	 */
	class StreamGobbler extends Thread {
		private final InputStream stream;
		private final String prefix;

		StreamGobbler(InputStream stream, String prefix) {
			this.stream = stream;
			this.prefix = prefix;
		}

		public void run() {
			try {
				BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
				String line;
				while ((line = reader.readLine()) != null)
					System.err.println(prefix + "> " + line);
			} catch (IOException e) {
				// It's fine if this thread crashes, nothing depends on it
				e.printStackTrace();
			}
		}
	}

	/**
	 * Starts the process and creates buffered/whatnot streams for stdin stderr or the external program
	 * @throws IOException if the process could not be started (see ProcessBuilder.start for details).
	 */
	private void setupProcess() throws IOException {
		process = pb.start();
		processInput = new OutputStreamWriter(process.getOutputStream());
		processOutput = new BufferedReader(new InputStreamReader(process.getInputStream()));
		errorGobbler = new StreamGobbler(process.getErrorStream(), "ERROR> main");
		errorGobbler.start();
	}

	/**
	 * I thought this might be a good idea, but I'm not a native Java speaker, so maybe it's not needed.
	 */
	private void closeAll() {
		// Since we're closing, I think it's ok to continue in case of an exception
		try {
			processInput.close();
			processOutput.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			errorGobbler.join(10);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		process.destroy();
		process = null;
		processInput = null;
		processOutput = null;
		errorGobbler = null;
	}

	/**
	 * Uses an external program to find counterexamples. The hypothesis will be written to stdin. Then the external
	 * program might do some calculations and write its test suite to stdout. This is in turn read here and fed
	 * to the SUL. If a discrepancy occurs, an counterexample is returned. If the external program exits (with value
	 * 0), then no counterexample is found, and the hypothesis is correct.
	 *
	 * This method might throw a RuntimeException if the external program crashes (which it shouldn't of course), or if
	 * the communication went wrong (for whatever IO reason).
	 */
	@Nullable
	@Override
	public DefaultQuery<String, Word<O>> findCounterExample(MealyMachine<?, String, ?, O> machine, Collection<? extends String> inputs) {
		// we're ignoring the external alphabet, only our own are used!
		while(true) {
			// start where we left previously
			for(; currentAlphabet < alphabets.size(); ++currentAlphabet){
				System.err.println("ERROR> log> Testing with subalphabet " + currentAlphabet);
				Alphabet<String> a = alphabets.get(currentAlphabet);
				DefaultQuery<String, Word<O>> r = findCounterExampleImpl(machine, a, bound);
				if (r != null) return r;
			}
			System.err.println("ERROR> log> Increasing bound by a factor of 10");
			currentAlphabet = 0;
			bound *= 10;
		}
	}

	private DefaultQuery<String, Word<O>> findCounterExampleImpl(MealyMachine<?, String, ?, O> machine, Collection<? extends String> inputs, long bound) {
		try {
			setupProcess();
		} catch (IOException e) {
			throw new RuntimeException("Unable to start the external program: " + e);
		}

		long queryCount = 0;
		try {
			// Write the hypothesis to stdin of the external program
			GraphDOT.write(machine, inputs, processInput);
			processInput.flush();

			// Read every line outputted on stdout.
			String line;
			while ((line = processOutput.readLine()) != null) {
				// Break if we did not fin one in time
				++queryCount;
				if(queryCount > bound) {
					System.err.println("ERROR> log> Bound is reached");
					closeAll();
					return null;
				}

				// Read every string of the line, this will be a symbol of the input sequence.
				WordBuilder<String> wb = new WordBuilder<>();
				Scanner s = new Scanner(line);
				while(s.hasNext()) {
					wb.add(s.next());
				}

				// Convert to a word and test on the SUL
				Word<String> test = wb.toWord();
				DefaultQuery<String, Word<O>> query = new DefaultQuery<>(test);
				sulOracle.processQueries(Collections.singleton(query));

				// Also test on the hypothesis
				Word<O> o1 = machine.computeOutput(test);
				Word<O> o2 = query.getOutput();

				assert o1 != null;
				assert o2 != null;

				// If equal => no counterexample :(
				if(o1.equals(o2)) continue;

				// If not equal => counterexample :D
				closeAll();
				return query;
			}
		} catch (IOException e) {
			throw new RuntimeException("Unable to communicate with the external program: " + e);
		}

		// At this point, the external program closed its stream, so it should have exited.
		if(process.isAlive()){
			System.err.println("ERROR> log> No counterexample but process stream still active!");
			closeAll();
			throw new RuntimeException("No counterexample but process stream still active!");
		}

		// If the program exited with a non-zero value, something went wrong (for example a segfault)!
		int ret = process.exitValue();
		if(ret != 0){
			System.err.println("ERROR> log> Something went wrong with the process: return value = " + ret);
			closeAll();
			throw new RuntimeException("Something went wrong with the process: return value = " + ret);
		}

		// Here, the program exited normally, without counterexample, so we may return null.
		return null;
	}
}
