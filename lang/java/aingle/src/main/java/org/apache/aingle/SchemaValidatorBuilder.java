/*

 */

package org.apache.aingle;

/**
 * <p>
 * A Builder for creating SchemaValidators.
 * </p>
 */
public final class SchemaValidatorBuilder {
  private SchemaValidationStrategy strategy;

  public SchemaValidatorBuilder strategy(SchemaValidationStrategy strategy) {
    this.strategy = strategy;
    return this;
  }

  /**
   * Use a strategy that validates that a schema can be used to read existing
   * schema(s) according to the AIngle default schema resolution.
   */
  public SchemaValidatorBuilder canReadStrategy() {
    this.strategy = new ValidateCanRead();
    return this;
  }

  /**
   * Use a strategy that validates that a schema can be read by existing schema(s)
   * according to the AIngle default schema resolution.
   */
  public SchemaValidatorBuilder canBeReadStrategy() {
    this.strategy = new ValidateCanBeRead();
    return this;
  }

  /**
   * Use a strategy that validates that a schema can read existing schema(s), and
   * vice-versa, according to the AIngle default schema resolution.
   */
  public SchemaValidatorBuilder mutualReadStrategy() {
    this.strategy = new ValidateMutualRead();
    return this;
  }

  public SchemaValidator validateLatest() {
    valid();
    return new ValidateLatest(strategy);
  }

  public SchemaValidator validateAll() {
    valid();
    return new ValidateAll(strategy);
  }

  private void valid() {
    if (null == strategy) {
      throw new AIngleRuntimeException("SchemaValidationStrategy not specified in builder");
    }
  }

}
