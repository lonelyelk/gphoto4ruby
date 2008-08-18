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
    puts "files on camera: " + c.files.inspect
    puts "some folder stuff: " + c.folder_up.subfolders.inspect
    c.capture.save :type => :preview, :new_name => "PREVIEW.JPG"
else
    puts ports.length.to_s + "cameras connected"
    cams = []
    ports.each do |port|
        c = GPhoto2::Camera.new(port)
        puts "camera in port: " + port
        c.configs.each do |cfg|
            puts cfg + " value is: " + c[cfg].to_s
            puts "values available are: " + c[cfg, :all].inspect
        end
        c.capture
        puts "files on camera: " + c.files.inspect
        puts "some folder stuff: " + c.folder_up.subfolders.inspect
        cams.push c
    end
    # to capture image with all attached cameras at a time use:
    cams.each_index do |index|
        if index < cams.length - 1
            fork {cams[index].capture; exit!}
        else
            cams[index].capture
        end
    end
end
