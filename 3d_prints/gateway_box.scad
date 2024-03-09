$fn=180;
//$fn=80;

width = 28;
length = 52;
height = 20;
wall_thickness = 1.7;

board_height = 7;
board_width = 25.5;
board_thickness = 1.7;

ir_sensor_r = 9.3/2;//9/2, 9.6/2

antenna_r = (9.3/2) + 0.4;
antenna_r2 = (6.3/2) + 0.2;

usb_c_w = 9.2;//8.45;
usb_c_h = 3.5;//2.65;

antenna_len = 5;

snap_r = 0.8; // 1

cover_len = length+ 0;
cover_wall_thickness = 1;

draw_cover = false;


module usb_buttons_holes()
{
    r_m = 1;
    h1 = usb_c_h-2*r_m;
    
    translate([width/2 - usb_c_w/2, length - 1, board_height - usb_c_h])
    {        
        translate([r_m, 0, r_m])
        {   
            minkowski(){
                
            cube(size = [usb_c_w-2*r_m, 30, usb_c_h-2*r_m], center = false);
                sphere(r_m);
            }
        }        
        
        //cube(size = [usb_c_w, 30, usb_c_h], center = false);
    }    
}


module antenna_holder_cut()
{
    rotate([-90,0,90])
        {   
            translate([42 + wall_thickness, -(height/2) - 4, 0])
            {
                cylinder(h=antenna_len-wall_thickness, r1=antenna_r, r2=antenna_r, center=false, $fn=6);
                
                cylinder(h=antenna_len*2, r1=antenna_r2, r2=antenna_r2, center=false);
            }
        }
}    

module antenna_holder()
{
    rotate([-90,0,90])
        {
            antenna_r1 = antenna_r + wall_thickness/2;
            antenna_len1 = antenna_len + wall_thickness;
            translate([42 + wall_thickness, -(height/2) - 4, 0])
            {
                cylinder(h=antenna_len, r1=antenna_r1, r2=antenna_r1, center=false);
            }
        }
}    


module display_cut()
{
    display_l = 27;
    display_w = 13;
    
    shift_w = 5;
    shift_l = 7;
    
    translate([shift_w + (width-board_width)/2, shift_l, -5])
    cube(size = [display_w, display_l, 8], center = false);
}



    shift_l = 3.5;
    shift_w = 4.5;
    
    btn_r = 3/2;
    
    btn_h = 4 - board_thickness;

module button()
{   
    cyl_h = board_height - btn_h - 0.1;
    
    translate([shift_w + (width-board_width)/2, length - shift_l, 0])
    {
    cylinder(h=cyl_h, r1=btn_r, r2=btn_r, center=false);
    }    
}

module button_cut()
{
    cut_thickness = 0.5;
    len = 8*btn_r;
    wid = 3*btn_r;
    // cut around the cylinder    
    translate([shift_w + (width-board_width)/2 - (wid)/2, length - shift_l - (len-btn_r), -wall_thickness*2])
    {
        difference()
        {
            translate([-(cut_thickness), cut_thickness, 0])
        {
            cube(size = [wid + 2*cut_thickness, len, 10], center = false);        
        }
        
       cube(size = [wid, len, 10], center = false);        
    }
    }       
}




module board_snap()
{
   
    translate([0,0, board_height - snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=(width/2) - 5, r1=snap_r, r2=snap_r, center=false);
        }
    }
    
    translate([width/2 + 5,0, board_height - snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=(width/2) - 5, r1=snap_r, r2=snap_r, center=false);
        }
    }
    
    
    translate([0,0, board_height + board_thickness + snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=width, r1=snap_r, r2=snap_r, center=false);
        }
    }
    
    /////
    
    translate([0,length, board_height - snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=(width/2) - 5, r1=snap_r, r2=snap_r, center=false);
        }
    }
    
    translate([width/2 + 5,length, board_height - snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=(width/2) - 5, r1=snap_r, r2=snap_r, center=false);
        }
    }  

}


module box0()
{
    union()
    {
    
    board_snap();       
    antenna_holder();  
    button();
        
     
    
    difference()
{
    
    union()
    {
    translate([ -wall_thickness, -wall_thickness, -wall_thickness])
    {
cube(size = [width + 2*wall_thickness, length + 2*wall_thickness, height + 2*wall_thickness], center = false);
    }  
    }
    
    
    union()
    {
   cube(size = [width, length, height + 10], center = false);
    }
    
}
}
}

module cover1()
{
    
    difference()
    {
        union()
        {            
            translate([-wall_thickness, -wall_thickness, height-1*wall_thickness])
            {
                cube(size = [width+2*wall_thickness, length + 2*wall_thickness, 3*wall_thickness], center = false);    
            }
        }

        translate([cover_wall_thickness, cover_wall_thickness, height-1*wall_thickness])
        cube(size = [width-2*cover_wall_thickness, 
                length-2*cover_wall_thickness, 
                2*wall_thickness], center = false);
    }
}


module cover_snaps()
{
    snap_rd = wall_thickness/2;
    snap_len = 5;
    
    translate([width/2 - snap_len/2,0, height ])
    {
        rotate([0,90,0])
        {
            cylinder(h=snap_len, r1=snap_rd, r2=snap_rd, center=false);
        }
    }
    
    translate([width/2 - snap_len/2,length, height ])
    {
        rotate([0,90,0])
        {
            cylinder(h=snap_len, r1=snap_rd, r2=snap_rd, center=false);
        }
    }
}




module all()
{
    union()
    {
        usb_buttons_holes();
        antenna_holder_cut();
        cover_snaps();
        button_cut();
        display_cut();
    }
}


if(draw_cover)
{    
    scale_ratio = 0.99;
    scale([scale_ratio, scale_ratio, scale_ratio])
    {
        difference()
        {
            cover1();
            box0();    
        }
        cover_snaps();
    }    
}else
{
    difference()
    {
        box0();    
        all();
    }
}







    






