/*

 */

package org.apache.aingle;

/**
 * <p>
 * A SchemaValidator has one method, which validates that a {@link Schema} is
 * <b>compatible<b/> with the other schemas provided.
 * </p>
 * <p>
 * What makes one Schema compatible with another is not part of the interface
 * contract.
 * </p>
 */
public interface SchemaValidator {

  /**
   * Validate one schema against others. The order of the schemas to validate
   * against is chronological from most recent to oldest, if there is a natural
   * chronological order. This allows some validators to identify which schemas
   * are the most "recent" in order to validate only against the most recent
   * schema(s).
   *
   * @param toValidate The schema to validate
   * @param existing   The schemas to validate against, in order from most recent
   *                   to latest if applicable
   * @throws SchemaValidationException if the schema fails to validate.
   */
  void validate(Schema toValidate, Iterable<Schema> existing) throws SchemaValidationException;

}
