#!/usr/bin/env ruby

require "rubygems"
require "rake"
require "rake/rdoctask"

desc "Generate RDoc documentation for gphoto4ruby gem."

Rake::RDocTask.new(:rdoc) do |rdoc|
    rdoc.rdoc_files.include("README.rdoc", "LICENSE", "CHANGELOG.rdoc").
            include("docs/COPYING", "docs/COPYING.LESSER").
            include("ext/gphoto2camera_event.c", "ext/gphoto2camera.c", "ext/gphoto4ruby.c")
    rdoc.main = "README.rdoc"
    rdoc.title = "GPhoto4Ruby documentation"
    rdoc.rdoc_dir = "rdoc"
    rdoc.options << "--charset=UTF-8"
    rdoc.options << "--webcvs=http://github.com/lonelyelk/gphoto4ruby/tree/master"
end

desc "Upload rdoc to RubyForge."

task :publish_rdoc => [:rdoc] do
  exec "scp -r rdoc/* lonelyelk@rubyforge.org:/var/www/gforge-projects/gphoto4ruby"
end

begin
  require 'jeweler'
  spec = Gem::Specification.new do |s|
    s.name = "gphoto4ruby"
    s.summary = "GPhoto4Ruby is Ruby wrapping around gphoto2 C library"
    s.description = "GPhoto4Ruby is used to control PPTP cameras (the ones that can be controlled with gphoto2) using power of ruby."

    s.authors = ["neq4 company", "Sergey Kruk"]
    s.email = "sergey.kruk@gmail.com"
    s.homepage = "http://github.com/lonelyelk/gphoto4ruby"
    s.rubyforge_project = "gphoto4ruby"
    
    s.has_rdoc = true
    s.rdoc_options << "--main" << "README.rdoc"
    s.rdoc_options << "--charset" << "UTF-8"
    s.rdoc_options << "--webcvs" << "http://github.com/lonelyelk/gphoto4ruby/tree/master"
    s.extra_rdoc_files = [ "ext/gphoto4ruby.c",
                    "ext/gphoto2camera.c", "ext/gphoto2camera_event.c",
                    "README.rdoc", "LICENSE", "CHANGELOG.rdoc",
                    "docs/COPYING", "docs/COPYING.LESSER"]
    
    s.files = %w(CHANGELOG.rdoc LICENSE README.rdoc Rakefile VERSION docs docs/COPYING docs/COPYING.LESSER example.rb ext ext/extconf.rb ext/gphoto4ruby.c ext/gphoto2camera.c ext/gphoto2camera.h ext/gphoto2camera_event.c ext/gphoto2camera_event.h ext/gphoto2camera_utilities.c ext/gphoto2camera_utilities.h)
    s.extensions = ["ext/extconf.rb"]
  end
  Jeweler::Tasks.new(spec)
  Jeweler::RubyforgeTasks.new
  Jeweler::GemcutterTasks.new
rescue LoadError
  puts "Jeweler (or a dependency) not available. Install it with: sudo gem install jeweler"
end
