/*

 */
package org.apache.aingle.tool;

import java.lang.reflect.Method;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;
import org.apache.aingle.file.Codec;
import org.apache.aingle.file.CodecFactory;
import org.apache.aingle.file.ZstandardCodec;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

public class TestUtil {

  @Rule
  public TestName name = new TestName();

  private void zstandardCompressionLevel(int level) throws Exception {
    OptionParser optParser = new OptionParser();
    OptionSpec<String> codecOpt = Util.compressionCodecOption(optParser);
    OptionSpec<Integer> levelOpt = Util.compressionLevelOption(optParser);

    OptionSet opts = optParser.parse(new String[] { "--codec", "zstandard", "--level", String.valueOf(level) });
    CodecFactory codecFactory = Util.codecFactory(opts, codecOpt, levelOpt);
    Method createInstance = CodecFactory.class.getDeclaredMethod("createInstance");
    createInstance.setAccessible(true);
    Codec codec = (ZstandardCodec) createInstance.invoke(codecFactory);
    Assert.assertEquals(String.format("zstandard[%d]", level), codec.toString());
  }

  @Test
  public void testCodecFactoryZstandardCompressionLevel() throws Exception {
    zstandardCompressionLevel(1);
    zstandardCompressionLevel(CodecFactory.DEFAULT_ZSTANDARD_LEVEL);
  }
}
