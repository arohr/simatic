ENV["RC_ARCHS"] = "" if RUBY_PLATFORM =~ /darwin/

# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

# Allow for custom compiler to be specified.
RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

# NOTE: use GCC flags unless Visual C compiler is used
$CFLAGS << ' -O2 -Wall -DDEBUG_CALLS -DLINUX -DDAVE_LITTLE_ENDIAN' unless RUBY_PLATFORM =~ /mswin/

if RUBY_VERSION < '1.8.6'
  $CFLAGS << ' -DRUBY_LESS_THAN_186'
end

# Do the work
# create_makefile(extension_name)
create_makefile('dave')
