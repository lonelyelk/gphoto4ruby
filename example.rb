require "rubygems"
require "gphoto4ruby"

ports = GPhoto2::Camera.ports
if ports.any?
    puts ports.length.to_s + "cameras connected"
    cams = []
    ports.each do |port|
        c = GPhoto2::Camera.new(port)
        puts "camera in port: " + port
        puts 'Model: ' + c.abilities[:model]
        c["capture"] = true if c.config.has_key? "capture" # canon? :)
        c.config(:no_cache).each do |key, value|
            puts key + " value is: " + value.to_s
            puts "values available are: " + c[key, :all].inspect
        end
        cams.push c
    end

    # capture image
    cams.first.capture
    
    # now camera virtual path is in the folder with images
    # list image file names
    puts "files on camera: " + cams.first.files.inspect
    
    # just an example of camera browsing
    puts "some folder stuff: " + cams.first.folder_up.subfolders.inspect
    
    # save preview of captured image in the current directory on hard drive
    cams.first.capture.save :type => :preview, :new_name => "PREVIEW.JPG"
    
    # save captured file in the current directory on hard drive and delete
    # it from camera
    cams.first.capture.save.delete
end
