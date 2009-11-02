require "mkmf"

if have_library("gphoto2")
    if RUBY_VERSION =~ /1\.9/
        $CPPFLAGS += " -DRUBY_19"
    end
    $CFLAGS = '-Wall ' + $CFLAGS
    create_makefile("gphoto4ruby")
else
    raise "You need gphoto2 installed to compile and use this library"
end
