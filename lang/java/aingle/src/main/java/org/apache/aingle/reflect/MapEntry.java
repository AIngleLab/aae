/*

 */
package org.apache.aingle.reflect;

import java.util.Map;

/**
 * Class to make AIngle immune from the naming variations of key/value fields
 * among several {@link java.util.Map.Entry} implementations. If objects of this
 * class are used instead of the regular ones obtained by
 * {@link Map#entrySet()}, then we need not worry about the actual field-names
 * or any changes to them in the future.<BR>
 * Example: {@code ConcurrentHashMap.MapEntry} does not name the fields as key/
 * value in Java 1.8 while it used to do so in Java 1.7
 *
 * @param <K> Key of the map-entry
 * @param <V> Value of the map-entry
 */
public class MapEntry<K, V> implements Map.Entry<K, V> {

  K key;
  V value;

  public MapEntry(K key, V value) {
    this.key = key;
    this.value = value;
  }

  @Override
  public K getKey() {
    return key;
  }

  @Override
  public V getValue() {
    return value;
  }

  @Override
  public V setValue(V value) {
    V oldValue = this.value;
    this.value = value;
    return oldValue;
  }
}
