$fn=180;
//$fn=80;

board_height = 5;
board_thickness = 1.1;

width = 25.4; //28
length = 41; //41
height = 24;
wall_thickness = 1.7;


ir_sensor_r = 9.3/2;//9/2, 9.6/2

antenna_r = (9.3/2) + 0.4;
antenna_r2 = (6.3/2) + 0.2;

usb_c_w = 9.2;//8.45;
usb_c_h = 3.5;//2.65;

antenna_len = 5;

snap_r = 1; // 1

cover_len = length+ 0;
cover_wall_thickness = 1;

draw_cover = true;


module cover(error=0.0)
{        
    translate([0,0,height])
    {
    cube(size = [width, cover_len, wall_thickness], center = false);
    }
    
    fin_r1 = (wall_thickness/2)/2;
    //fin_h = 10;
    
    h1 = height + wall_thickness/2;
    
    
    fin_s = wall_thickness;
    fin_len = 3 - 2*error;
    fin_h = wall_thickness/2 - error;
    
   
    translate([-fin_s,cover_len/3 + error,height])
    {
    cube(size = [fin_s, fin_len, fin_h], center = false);
    }
    
    translate([-fin_s,cover_len - cover_len/3 + error,height])
    {
    cube(size = [fin_s, fin_len, fin_h], center = false);
    }
    
    translate([width,cover_len/3 + error,height])
    {
    cube(size = [fin_s, fin_len, fin_h], center = false);
    }
    
    translate([width,cover_len - cover_len/3 + error,height])
    {
    cube(size = [fin_s, fin_len, fin_h], center = false);
    }
    
    
    translate([width/2 - fin_len/2,-fin_s,height])
    {
    cube(size = [fin_len, fin_s, fin_h], center = false);
    }
    
    translate([width/2 - fin_len/2,cover_len,height])
    {
    cube(size = [fin_len, fin_s, fin_h], center = false);
    }

}

module inner_legs()
{
    inner_leg_w = 2;
    inner_leg_h = height;
    
    cube(size = [inner_leg_w, inner_leg_w, inner_leg_h], center = false);
    
    translate([width - inner_leg_w,0,0])
    {
    cube(size = [inner_leg_w, inner_leg_w, inner_leg_h], center = false);
    }
    
    translate([width - inner_leg_w, length - inner_leg_w,0])
    {
    cube(size = [inner_leg_w, inner_leg_w, inner_leg_h], center = false);
    }
    
    translate([0, length - inner_leg_w,0])
    {
    cube(size = [inner_leg_w, inner_leg_w, inner_leg_h], center = false);
    }
}

module usb_buttons_holes()
{
    translate([width/2 - usb_c_w/2, length - 1, board_height + board_thickness])
    {   
        r_m = 1;
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

module ir_sensor()
{
    ir_sensor_r2 = ir_sensor_r+1;
    translate([width/2,0, board_height + board_thickness+ir_sensor_r2+snap_r+3-(ir_sensor_r2-ir_sensor_r)])
{
rotate([90,0,0])
        {
    cylinder(h=wall_thickness+5, r1=ir_sensor_r2, r2=ir_sensor_r2, center=false);
        }
    }
}

module ir_sensor_cut()
{
    translate([width/2,0, board_height + board_thickness+ir_sensor_r+snap_r+3])
{
rotate([90,0,0])
        {
    cylinder(h=10, r1=ir_sensor_r, r2=ir_sensor_r, center=false);
        }
    }
}

module antenna_holder_cut()
{
    rotate([-90,0,90])
        {   
            translate([20 + wall_thickness, -(height/2) - 4, 0])
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
            translate([20 + wall_thickness, -(height/2) - 4, 0])
            {
                cylinder(h=antenna_len, r1=antenna_r1, r2=antenna_r1, center=false);
            }
        }
}    

module button_cut1()
{
    btn_r = 3/2;
    cut_thickness = 0.5;
    len = 8*btn_r;
    wid = 3*btn_r;
    // cut around the cylinder    
    translate([width-5, length - len - 4, board_height])
    {
        difference()
        {
            translate([0, cut_thickness, -(cut_thickness)])
            {
                cube(size = [10, len, wid + 2*cut_thickness], center = false);        
            }
        
            cube(size = [10, len, wid], center = false);        
        }
    }       
}


module button_cut()
{
    btn_r = 3/2;
    cut_thickness = 0.5;
    len = 6*btn_r;
    wid = 3*btn_r;
    // cut around the cylinder    
    translate([width-5, length - wid - 3, board_height - len + wall_thickness + 2])
    {
        difference()
        {
            translate([0, -(cut_thickness), cut_thickness])
            {
                cube(size = [10, wid + 2*cut_thickness, len], center = false);        
            }
        
            cube(size = [10, wid, len], center = false);        
        }
    }       
}


module board_snap()
{
   
    translate([0,0, board_height - snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=width, r1=snap_r, r2=snap_r, center=false);
        }
    }
    
    translate([0,0, board_height + board_thickness + snap_r])
    {
    rotate([0,90,0])
        {
    //cylinder(h=width, r1=snap_r, r2=snap_r, center=false);
        }
    }
    
    
    translate([0,length, board_height - snap_r])
    {
    rotate([0,90,0])
        {
    cylinder(h=width, r1=snap_r, r2=snap_r, center=false);
        }
    }
}


module box0()
{
    union()
    {
    
    board_snap();   
    
    antenna_holder();
        
        ir_sensor();
        
        //inner_legs();
        
     
    
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
    translate([-wall_thickness, -wall_thickness, height-1*wall_thickness])
{
    cube(size = [width+2*wall_thickness, length + 2*wall_thickness, 3*wall_thickness], center = false);    
}

translate([cover_wall_thickness, cover_wall_thickness, height-1*wall_thickness])

    cube(size = [width-2*cover_wall_thickness, 
                length-2*cover_wall_thickness, 
                2*wall_thickness], center = false);
}

}





module all()
{
    union()
    {
        ir_sensor_cut();
        usb_buttons_holes();
        antenna_holder_cut();
        cover_snaps();
        button_cut();
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
    cylinder(h=5, r1=snap_rd, r2=snap_rd, center=false);
        }
    }
    
translate([width/2 - snap_len/2,length, height ])
    {
    rotate([0,90,0])
        {
    cylinder(h=5, r1=snap_rd, r2=snap_rd, center=false);
        }
    }
}

if(draw_cover)
{    
    scale_ratio = 0.995;
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








    






