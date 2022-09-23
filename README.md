<pre>
  ___ ___  _ __ ___  _ __ ___   ___  _ __
 / __/ _ \| '_ ` _ \| '_ ` _ \ / _ \| '_ \
| (_| (_) | | | | | | | | | | | (_) | | | |
 \___\___/|_| |_| |_|_| |_| |_|\___/|_| |_|
</pre>

# Common code to be reused in other projects



### Install cppunit for unit testing C++

Debian/Ubuntu:
<pre>
$ sudo apt-get install libcppunit-doc libcppunit-dev
</pre>

### Install OpenSSL dev for implementation of secure sockets

To build this library you will also need to install the OpenSSL dev package
and the libxml2 dev package.

cm_ssl uses OpenSSL for the implementation of secure sockets.
cm_xml uses libxml2 to implement an xml_reader.


Debian/Ubuntu:
<pre>
$ sudo apt-get install libssl-dev
$ sudo apt install libxml2-dev
</pre>

Rasbian:
<pre>
$ sudo apt-get install libssl-dev
$ sudo apt install libxml2-dev
</pre>

