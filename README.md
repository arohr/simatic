# simatic

A Ruby communication library for Siemens Simatic S7 PLCs. Can talk to:

* CP-343 over ISO-TCP with [libnodave](http://sourceforge.net/projects/libnodave/) binding
* CP-341 over RS-232 with [rkeo](http://code.google.com/p/rkeo/) binding

Can be used with EventMachine for communication with multiple PLCs. 

# Example

```ruby
#!/usr/bin/env ruby

require 'rubygems'
require 'simatic'
require 'dave'

dave = Dave.new '10.1.2.3', 102

loop do

  begin
    dave.connect
    puts dave.connected?
    data = dave.fetch 80, 1
    puts data.inspect
  rescue SocketError => e
    puts e.message
  ensure
    dave.disconnect
  end

  sleep 1

end
```