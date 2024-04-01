escpos_parser
===

Easily extract and understand data from thermal receipt printers with this C library. It parses generic ESC/POS raw binary data, a common format used by many receipt printers, making it ideal for developers working with digitalizing receipt information.

## How to use

This library simplifies working with ESC/POS data from thermal receipt printers. Here's the process:

1. Define your printer's command set: Identify the specific ESC/POS commands supported by your target printer model.
2. Parse the raw data: Use the library to parse the raw binary data extracted from the receipt printer. This process will break down the data into individual ESC/POS tokens.
3. Process the tokens: Iterate through the extracted tokens to achieve your desired outcome. This could involve tasks like image rendering, PDF conversion, or other data manipulation based on your project's needs.

For a more detailed example, please refer to the provided example code.

## Libraries used

* https://github.com/swenson/vector.h
* https://troydhanson.github.io/uthash
* https://github.com/codeplea/tinyexpr

## License

Copyright 2024 @ thaolt@songphi.com https://github.com/thaolt/escpos_parser

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
