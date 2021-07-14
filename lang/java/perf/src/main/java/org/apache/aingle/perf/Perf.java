/*

 */

package org.apache.aingle.perf;

import java.io.PrintWriter;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.options.ChainedOptionsBuilder;
import org.openjdk.jmh.runner.options.OptionsBuilder;
import org.openjdk.jmh.runner.options.TimeValue;
import org.openjdk.jmh.runner.options.WarmupMode;

/**
 * Performance tests for various low level operations of AIngle encoding and
 * decoding.
 */
public final class Perf {

  public static void main(String[] args) throws Exception {
    Options options = new Options();
    options.addOption(Option.builder().argName("measurementIterations").longOpt("mi").hasArg()
        .desc("The number of measure iterations").numberOfArgs(1).build());

    options.addOption(Option.builder().argName("warmupIterations").longOpt("wi").hasArg()
        .desc("The number of warmup iterations").numberOfArgs(1).build());

    options.addOption(Option.builder().argName("bulkWarmup").longOpt("bw").desc("Flag to enabled bulk warmup").build());

    options.addOption(
        Option.builder().argName("test").longOpt("test").hasArg().desc("The performance tests to run").build());

    options.addOption(Option.builder().argName("help").longOpt("help").desc("Print the help menu").build());

    final CommandLine cmd = new DefaultParser().parse(options, args);

    if (cmd.hasOption("help")) {
      final HelpFormatter formatter = new HelpFormatter();
      final PrintWriter pw = new PrintWriter(System.out);
      formatter.printUsage(pw, 80, "Perf", options);
      pw.flush();
      return;
    }

    String[] tests = cmd.getOptionValues("test");
    if (tests == null || tests.length == 0) {
      tests = new String[] { Perf.class.getPackage().getName() + ".*" };
    }

    final Integer measurementIterations = Integer.valueOf(cmd.getOptionValue("mi", "3"));
    final Integer warmupIterations = Integer.valueOf(cmd.getOptionValue("wi", "3"));

    final ChainedOptionsBuilder runOpt = new OptionsBuilder().mode(Mode.Throughput).timeout(TimeValue.seconds(60))
        .warmupIterations(warmupIterations).measurementIterations(measurementIterations).forks(1).threads(1)
        .shouldDoGC(true);

    if (cmd.hasOption("builkWarmup")) {
      runOpt.warmupMode(WarmupMode.BULK);
    }

    for (final String test : tests) {
      runOpt.include(test);
    }

    new Runner(runOpt.build()).run();
  }
}
