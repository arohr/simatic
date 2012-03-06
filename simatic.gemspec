# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)

Gem::Specification.new do |s|
  s.name        = "simatic"
  s.version     = "0.0.2"
  s.platform    = Gem::Platform::RUBY
  s.authors     = ["Kiril Kirov"]
  s.email       = ["kiril@kirov.be"]
  s.homepage    = "https://github.com/kirilk/simatic"
  s.summary     = %q{A Ruby communication library for Siemens Simatic S7 PLCs}
  s.description = %q{A Ruby communication library for Siemens Simatic S7 PLCs}
  s.rdoc_options = ["--main", "README.md"]

  s.add_dependency 'rake'
  s.add_dependency 'bitstring'

  s.add_development_dependency 'rspec', '>= 2.5.0'
  s.add_development_dependency 'rake-compiler'

  s.files         = `git ls-files`.split("\n")
  s.extensions    = ["ext/dave/extconf.rb", "ext/rkeo/extconf.rb"]
  s.require_paths = ["lib"]
end
