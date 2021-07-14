/*

 */
package org.apache.aingle.io;

import java.io.IOException;

import org.apache.aingle.io.parsing.SkipParser;
import org.apache.aingle.io.parsing.Symbol;
import org.apache.aingle.io.parsing.Parser.ActionHandler;
import org.apache.aingle.io.parsing.SkipParser.SkipHandler;

/**
 * Base class for <a href="parsing/package-summary.html">parser</a>-based
 * {@link Decoder}s.
 */
public abstract class ParsingDecoder extends Decoder implements ActionHandler, SkipHandler {
  protected final SkipParser parser;

  protected ParsingDecoder(Symbol root) throws IOException {
    this.parser = new SkipParser(root, this, this);
  }

  protected abstract void skipFixed() throws IOException;

  @Override
  public void skipAction() throws IOException {
    parser.popSymbol();
  }

  @Override
  public void skipTopSymbol() throws IOException {
    Symbol top = parser.topSymbol();
    if (top == Symbol.NULL) {
      readNull();
    } else if (top == Symbol.BOOLEAN) {
      readBoolean();
    } else if (top == Symbol.INT) {
      readInt();
    } else if (top == Symbol.LONG) {
      readLong();
    } else if (top == Symbol.FLOAT) {
      readFloat();
    } else if (top == Symbol.DOUBLE) {
      readDouble();
    } else if (top == Symbol.STRING) {
      skipString();
    } else if (top == Symbol.BYTES) {
      skipBytes();
    } else if (top == Symbol.ENUM) {
      readEnum();
    } else if (top == Symbol.FIXED) {
      skipFixed();
    } else if (top == Symbol.UNION) {
      readIndex();
    } else if (top == Symbol.ARRAY_START) {
      skipArray();
    } else if (top == Symbol.MAP_START) {
      skipMap();
    }
  }

}
