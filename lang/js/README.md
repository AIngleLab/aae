<!--
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at



Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->


# AIngle-js

Pure JavaScript implementation of the [AIngle specification](https://apache.aingle.ai/docs/current/spec.html).


## Features

+ Fast! Typically twice as fast as JSON with much smaller encodings.
+ Full AIngle support, including recursive schemas, sort order, and evolution.
+ Serialization of arbitrary JavaScript objects via logical types.
+ Unopinionated 64-bit integer compatibility.
+ No dependencies, `aingle-js` even runs in the browser.


## Installation

```bash
$ npm install aingle-js
```

`aingle-js` is compatible with all versions of [node.js][] since `0.11` and major
browsers via [browserify][].


## Documentation

See `doc/` folder.


## Examples

Inside a node.js module, or using browserify:

```javascript
var aingle = require('aingle-js');
```

+ Encode and decode objects:

  ```javascript
  // We can declare a schema inline:
  var type = aingle.parse({
    name: 'Pet',
    type: 'record',
    fields: [
      {name: 'kind', type: {name: 'Kind', type: 'enum', symbols: ['CAT', 'DOG']}},
      {name: 'name', type: 'string'}
    ]
  });
  var pet = {kind: 'CAT', name: 'Albert'};
  var buf = type.toBuffer(pet); // Serialized object.
  var obj = type.fromBuffer(buf); // {kind: 'CAT', name: 'Albert'}
  ```

+ Generate random instances of a schema:

  ```javascript
  // We can also parse a JSON-stringified schema:
  var type = aingle.parse('{"type": "fixed", "name": "Id", "size": 4}');
  var id = type.random(); // E.g. Buffer([48, 152, 2, 123])
  ```

+ Check whether an object fits a given schema:

  ```javascript
  // Or we can specify a path to a schema file (not in the browser):
  var type = aingle.parse('./Person.ain');
  var person = {name: 'Bob', address: {city: 'Cambridge', zip: '02139'}};
  var status = type.isValid(person); // Boolean status.
  ```

+ Get a [readable stream][readable-stream] of decoded records from an AIngle
  container file (not in the browser):

  ```javascript
  aingle.createFileDecoder('./records.aingle')
    .on('metadata', function (type) { /* `type` is the writer's type. */ })
    .on('data', function (record) { /* Do something with the record. */ });
  ```


[node.js]: https://nodejs.org/en/
[readable-stream]: https://nodejs.org/api/stream.html#stream_class_stream_readable
[browserify]: http://browserify.org/
