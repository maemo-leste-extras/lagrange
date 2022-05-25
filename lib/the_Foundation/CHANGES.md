# Changelog

## 1.4 - Unreleased
* Block: Fixed crash when checking size of zero-initialized instance.
* Char: Added `width` method.

## 1.3 - 2022-04-05
* Archive: Opening as writable, serializing with compression.
* Block: Handle possible vprintf encoding errors.
* String: Upper/lower-casing in a specific language.
* Time: Added `max` method.
* TlsCertificate: Avoid repeated verification of the same certificate.
* TlsRequest: TLS session cache can be disabled per request.

## 1.2.1
* SOVERSION includes the minor version, since new symbols may be added in minor releases.

## 1.2 - 2022-03-05
* String: Added replacing/substituting with a regular expression.
* TlsCertificate: Fixed copying of X509 chains.
* TlsRequest: Added a session cache to avoid repeat handshakes.

## 1.1 - 2022-01-20
* Detect Android as a variant of Linux. Some features like starting child processes are disabled in Android.
* Block: gzip-compatible decompression.
* Char: Querying script of a Unicode code point.
* Process: Remember exit status code.

## 1.0.3
* TlsRequest: Set "valid from" time using UTC instead of local time.

## 1.0 - 2021-11-06
* Initial release.