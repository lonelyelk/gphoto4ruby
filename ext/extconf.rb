require "mkmf"

lib_gp_paths = dir_config("gphoto2")

if find_library("gphoto2", nil, *lib_gp_paths)
    if RUBY_VERSION =~ /1\.9/
        $CPPFLAGS += " -DRUBY_19"
    end
    $CFLAGS = '-Wall ' + $CFLAGS
    create_makefile("gphoto4ruby")
else
    raise "You need libgphoto2 installed to compile and use this library"
end
