import aingle.schema
from aingle.datafile import DataFileReader, DataFileWriter
from aingle.io import DatumReader, DatumWriter

schema = aingle.schema.parse(open("user.ain").read())

writer = DataFileWriter(open("/tmp/users.aingle", "w"), DatumWriter(), schema)
writer.append({"name": "Alyssa", "favorite_number": 256, "WTF": 2})
writer.append({"name": "Ben", "favorite_number": 7, "favorite_color": "red"})
writer.close()

reader = DataFileReader(open("/tmp/users.aingle", "r"), DatumReader())
for user in reader:
    print user
reader.close()
