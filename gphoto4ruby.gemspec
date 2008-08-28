Gem::Specification.new do |s|
    s.name = "gphoto4ruby"
    s.summary = "GPhoto4Ruby is Ruby wrapping around libgphoto2 C library"

    s.version = "0.1.4"
    s.date = "2008-08-28"
    s.authors = ["heq4 company", "Sergey Kruk"]
    s.email = "sergey.kruk@gmail.com"
    s.homepage = "http://github.com/lonelyelk/gphoto4ruby"
    s.rubyforge_project = "gphoto4ruby"
    
    s.has_rdoc = true
    s.rdoc_options << "--main" << "README.rdoc"
    s.rdoc_options << "--charset" << "UTF-8"
    s.rdoc_options << "--webcvs" << "http://github.com/lonelyelk/gphoto4ruby/tree/master"
    s.extra_rdoc_files = [ "ext/gphoto4ruby.c",
                    "README.rdoc", "LICENSE", "CHANGELOG.rdoc",
                    "docs/COPYING", "docs/COPYING.LESSER"]
    
    s.files = %w(CHANGELOG.rdoc LICENSE README.rdoc Rakefile docs docs/COPYING docs/COPYING.LESSER example.rb ext ext/extconf.rb ext/gphoto4ruby.c ext/gphoto4ruby.h)
    s.extensions = ["ext/extconf.rb"]
end
