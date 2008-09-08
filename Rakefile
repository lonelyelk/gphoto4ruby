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
