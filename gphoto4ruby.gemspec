require "rake"

Gem::Specification.new do |s|
    s.name = "gphoto4ruby"
    s.summary = "GPhoto4Ruby is Ruby wrapping around libgphoto2 C library"

    s.version = "0.1.1"
    s.date = "2008-08-15"
    s.authors = ["heq4 company", "Sergey Kruk"]
    s.email = "sergey.kruk@gmail.com"
    s.homepage = "http://github.com/lonelyelk/gphoto4ruby"
    s.rubyforge_project = "gphoto4ruby"
    
    s.has_rdoc = true
    s.rdoc_options << "--main" << "README.rdoc"
    s.rdoc_options << "--charset" << "UTF-8"
    s.rdoc_options << "--webcvs" << "http://github.com/lonelyelk/gphoto4ruby/tree/master"
    s.extra_rdoc_files = ["README.rdoc", "LICENSE", "CHANGELOG.rdoc", "ext/gphoto4ruby.c"]
    
    s.files = FileList["ext/*.c", "ext/*.h", "docs/*", "[A-Z]*", "example.rb"].to_a
    s.extensions = ["ext/extconf.rb"]
end
