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

# connect to 10.1.2.3:102 and use slot 2 (eg. S7-300). Use slot 0 for S7-1200.
dave = Dave.new '10.1.2.3', 102, 2

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