/*

 */

package example;

import java.io.File;
import java.io.IOException;

import org.apache.aingle.file.DataFileReader;
import org.apache.aingle.file.DataFileWriter;
import org.apache.aingle.io.DatumReader;
import org.apache.aingle.io.DatumWriter;
import org.apache.aingle.specific.SpecificDatumReader;
import org.apache.aingle.specific.SpecificDatumWriter;

import example.aingle.User;

public class SpecificMain {
  public static void main(String[] args) throws IOException {
    User user1 = new User();
    user1.setName("Alyssa");
    user1.setFavoriteNumber(256);
    // Leave favorite color null

    // Alternate constructor
    User user2 = new User("Ben", 7, "red");

    // Construct via builder
    User user3 = User.newBuilder()
             .setName("Charlie")
             .setFavoriteColor("blue")
             .setFavoriteNumber(null)
             .build();

    // Serialize user1 and user2 to disk
    File file = new File("users.aingle");
    DatumWriter<User> userDatumWriter = new SpecificDatumWriter<User>(User.class);
    DataFileWriter<User> dataFileWriter = new DataFileWriter<User>(userDatumWriter);
    dataFileWriter.create(user1.getSchema(), file);
    dataFileWriter.append(user1);
    dataFileWriter.append(user2);
    dataFileWriter.append(user3);
    dataFileWriter.close();

    // Deserialize Users from disk
    DatumReader<User> userDatumReader = new SpecificDatumReader<User>(User.class);
    User user = null;
    try(DataFileReader<User> dataFileReader = new DataFileReader<User>(file, userDatumReader)){
      while (dataFileReader.hasNext()) {
        // Reuse user object by passing it to next(). This saves us from
        // allocating and garbage collecting many objects for files with
        // many items.
        user = dataFileReader.next(user);
        System.out.println(user);
      }
    }
  }
}
