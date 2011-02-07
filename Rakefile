require 'rubygems'
require 'rake/extensiontask'
require 'rake/testtask'

Rake::ExtensionTask.new('dave')
Rake::ExtensionTask.new('rkeo')

Rake::TestTask.new do |t|
	t.test_files = FileList['test/*.rb']
	t.verbose = true
end
