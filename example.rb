require "rubygems"
require "gphoto4ruby"

ports = GPhoto2::Camera.ports
if ports.empty?

    # ports array can be empty if there is only one camera plugged in
    # or there are no cameras connected to the computer
    # assuming there is one
    c = GPhoto2::Camera.new()

    # list available configuration items with current values and lists
    # of allowed values
    c.config.each_key do |cfg_key|
        puts cfg_key + " value is: " + c.config[cfg_key].to_s # or c[cfg_key].to_s
        puts "values available are: " + c[cfg_key, :all].inspect
    end

    # capture image
    c.capture
    
    # now camera virtual path is in the folder with images
    # list image file names
    puts "files on camera: " + c.files.inspect
    
    # just an example of camera browsing
    puts "some folder stuff: " + c.folder_up.subfolders.inspect
    
    # save preview of captured image in the current directory on hard drive
    c.capture.save :type => :preview, :new_name => "PREVIEW.JPG"
    
    # save captured file in the current directory on hard drive and delete
    # it from camera
    c.capture.save.delete
else
    puts ports.length.to_s + "cameras connected"
    cams = []
    ports.each do |port|
        c = GPhoto2::Camera.new(port)
        puts "camera in port: " + port
        c.config.each_key do |cfg_key|
            puts cfg_key + " value is: " + c[cfg_key].to_s
            puts "values available are: " + c[cfg_key, :all].inspect
        end
        cams.push c
    end
    # to capture image with all attached cameras simultaneously use:
    cams.each_index do |index|
        if index < cams.length - 1
            fork {cams[index].capture; exit!}
        else
            cams[index].capture
        end
    end
end
