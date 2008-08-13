require "rake"

Gem::Specification.new do |s|
    s.name = "gphoto4ruby"
    s.version = "0.1.0"
    s.authors = ["heq4 company", "Sergey Kruk"]
    s.email = "sergey.kruk@gmail.com"
    s.summary = "GPhoto4Ruby is Ruby wrapping for libgphoto2 C library"
    s.files = FileList["ext/*.c", "ext/*.h", "docs/*", "[A-Z]*", "example.rb"].to_a
    s.extensions = ["ext/extconf.rb"]
end
