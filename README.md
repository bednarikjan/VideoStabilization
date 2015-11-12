# VideoStabilization
This project was the main deliverable of the course Image Processing at Faculty of Information Technology, Brno University of Technology. The aim was to implement a video stabilizer. The resulting program can be used as a command line tool taking the input video and producing the output stabilized video.

## How it works?

Two separate methods are implemented. The first method, Gray Coded Bit Plane Matching, compares the bit fields in the consecutive frames of the video and tries to estimate the most likely translation. The second method utilizes the KLT tracker for detection and tracking of the key points and SVD for obtaining the rotation and translation matrices for the motion compensation. For more details, see [Video stabilization](http://bednarikjan.github.io/2015/06/05/trajectory_clustering.html).

## Install and run

```
$ make
$ ./dis [options]
```

## Synopsis
    dis -i filename [options]
        
    OPTIONS
        -i filename
            input file name
            
        -o filename
            output file name
    
        -s stabilizer
            sets stabilization method to stabilizer. Available methods are:
            
            gcbpm (default)
            svd
            
        -m method
            sets method of producing the output video. Available methods are:
            
            raw (default)
                output video size is the same as input
            intersection
                output video size corresponds with the intersection of all frames
            union
                all frames all fully displayed so the resulting video size gets bigger
                
        -l
            if method union was chosen (see option -m) each frame is left in the video,
            the ones are placed on top of the old ones.
            
      
    OPTIONS relevant for stabilizer 'gcbpm'
        -b bit
            sets the n-th bit to be used in bit-planes. Allowed values: 0-8 (default 
            is 5).
            
        -N size
            sets size of the comparing window (which is square size x size) in pixels. 
            The larger value the better quality but slower run (default is 50). If the
            value is too big the biggest suitable smaller one will be set automatically.
            
        -f offset
            sets the offset (in both x and y direction) in which the comparing window
            will slide. The larger value the better quality but slower run 
            (default is 40). If the value is too big the biggest suitable smaller one 
            will be set automatically.
            
        -x subregions
            sets the number of search subregions whithin frame in x axis (default
            is 2)
            
        -y ubregions
            sets the number of search subregions whithin frame in yaxis (default
            is 2)
            
        -d damping
            sets the damping of the last global motion vector. The higher the value
            the less shaky resulting video will be but more likely to 'escape' the 
            rame. Allowed values (floating point): 0-1 (default is 0.95)
    
    OPTIONS relevant for stabilizer 'gcbpm'
        -c points
            sets the number of key points to track using KLT tracker
