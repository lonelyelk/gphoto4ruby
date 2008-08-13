require "mkmf"

dir_config("gphoto4ruby", "/usr/local")
if have_library("gphoto2")
    create_makefile("gphoto4ruby")
else
    raise "You need gphoto2 installed to compile and use this library"
end
