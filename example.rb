require "rubygems"
require "gphoto4ruby"

ports = GPhoto2::Camera.ports
if ports.empty?
    # ports array can be empty if there is only one camera plugged in
    # or there are no cameras connected to the computer
    # assuming there is one
    c = GPhoto2::Camera.new()
    c.configs.each do |cfg|
        puts cfg + " value is: " + c[cfg].to_s
        puts "values available are: " + c[cfg, :all].inspect
    end
    c.capture
else
    puts ports.length.to_s + "cameras connected"
    ports.each do |port|
        c = GPhoto2::Camera.new(port)
        puts "camera in port: " + port
        c.configs.each do |cfg|
            puts cfg + " value is: " + c[cfg].to_s
            puts "values available are: " + c[cfg, :all].inspect
        end
        c.capture
    end
end
