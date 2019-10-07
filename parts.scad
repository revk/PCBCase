// Functions for parts attached to PCB
// Copyright (c) 2019 Adrian Kennard, Andrews & Arnold Limited, see LICENSE file (GPL)

// Origins are top left of PCB, so typically translated -1,-1,1.6 from pcb() to allow for 1 mm margin on SVGs
// The stage parameter is used to allow these functions to be called for pcb(), and cut()
// Stage=0 is the PCB part
// Stage=1 adds to box base
// Stage=-1 cuts in to box base
// Only parts that expect to "stick out" from the case do anything for the cut stages

module posn(x,y,w,h,r,vx=0.2,vy=0.2,vz=0)
{ // Positioning and rotation and growing for placement errors
	s=sin(r);
	c=cos(r);
	translate([x+(s>0?h*s:0)+(c<0?-w*c:0),y+(c<0?-h*c:0)+(s<0?-w*s:0),0])
	rotate([0,0,r])
	{
		if(vx||vy||vz)
		{
			minkowski()
			{
				children();
				cube([vx?vx:0.001,vy?vy:0.001,vz?vz:0.001],center=true);
			}
		}else children();
	}
}

module pads(x,y,d=1.2,h=2.5,nx=1,dx=2.54,ny=1,dy=2.54)
{ // PCB pad, x/y are centre of pin
	for(px=[0:1:nx-1])
	for(py=[0:1:ny-1])
	translate([x+px*dx,y+py*dy,-0.001])
	cylinder(d1=3,d2=d,h=h+0.001,$fn=8);
}

module esp32(stage,x,y,r=0)
{ // Corner of main board of ESP32 18mm by 25.5mm
	posn(x,y,18,25.5,r,1,0.5,0) // Note left/right margin for placement
	{
		if(!stage)
		{
			cube([18,25.5,1]);	// Base PCB
    			translate([1,1,0])
    			cube([16,18,3]);		// Can
    			translate([-1,1,0])
    			cube([20,18,2]); // Solder
		}else{ // Cut
			translate([0,15.5,0])
			hull()
			{
				translate([0,0,stage])
				cube([18,10,1]);	// Base PCB
				translate([-10,0,stage*20])
				cube([18+20,10,1]);
			}
		}
	}
}

module screw(stage,x,y,r,n=2,d,w,h,yp,ys,s=3,pcb=1.6)
{ // Corner of outline
	posn(x,y,d*n,w,r)
	{
		if(!stage)
		{
			pads(d/2,yp?yp:w/2,1.2,3.5-pcb,n,d);
			// Body
			translate([0,0,-pcb-h])
			cube([d*n,w,h+0.001]);
			// Screws
			for(px=[0:1:n-1])
			translate([d/2+d*px,ys?ys:w/2,-pcb-20-h])
			cylinder(d=s,h=20.001);
			// Wires
			for(px=[0:1:n-1])
			translate([d/2+d*px-(d-1)/2,-20,-pcb-1-(d-1)])
			cube([d-1,20.001,d-1]);
		}else{ // Cut
			translate([0,-20,-pcb-1-(d-1)/2-0.5])
			hull()
			{
				translate([0,0,stage/2])
				cube([d*n,20,1]);
				translate([0,0,stage*20])
				cube([d*n,20,1]);
			}
		}
	}
}

module screw5mm(stage,x,y,r,n=2)
{ // 8.1mm wide, 10mm high, 5mm spacing, low profile screw terminals, e.g. RS 897-0843
	screw(stage,x,y,r,n,5,8.1,10);
}

module screw3mm5a(stage,x,y,r,n=2)
{ // 7mm wide, 8.5mm high, 3.5mm spacing, low profile screw terminals, e.g. RS 144-4314
	screw(stage,x,y,r,n,3.5,7,8.5);
}

module screw3mm5(stage,x,y,r,n=2)
{ // 7.2mm wide, 8.75mm high, 3.5mm spacing, low profile screw terminals, e.g. RS 790-1149
	screw(stage,x,y,r,n,3.5,7.2,8.75,3.7,3);
}


module d24v5f3(stage,x,y,r=0,pcb=1.6)
{ // Pololu regulator using only 3 pins
	if(!stage)
	posn(x,y,25.4*0.4,25.4*0.5,r)
	{
		translate([0,0,-pcb-2.8])
		cube([25.4*0.4,25.4*0.5,2.8]);
		pads(25.4*0.05,25.4*0.05,1.2,3,3);

	}
}

module milligrid(stage,x,y,r=0,n=2,pcb=1.6)
{ // eg RS part 6700927
	if(!stage)
	posn(x,y,2.6+n*2,6.4,r,0.4,0.4)
	{
		translate([0,0,-pcb-6.3])
		cube([2.6+n*2,6.4,6.3001]);
		// Wires
		translate([0.5,0.5,-pcb-6.3-20])
		cube([pcb+n*2,5.4,20.001]);
		// pads
		pads(2.3,2.2,1,1,n,2,2,2);
	}
}


module molex(stage,x,y,r=0,nx=1,ny=1,pcb=1.6)
{ // Simple molex pins
	if(!stage)
	posn(x,y,2.54*nx,2.54*ny,r)
	{
		translate([0,0,-pcb-2.54])
		cube([nx*2.54,ny*2.54,2.5401]);
		for(px=[0:1:nx-1])
		for(py=[0:1:ny-1])
		translate([px*nx+2.54/2-0.5,py*ny+2.54/2-0.5,-9])
		cube([1,1,9.001]);
		pads(2.54/2,2.54/2,1,3.5-1.6,nx,2.54,ny,2.54);
	}
}

module smd1206(stage,x,y,r=0)
{ // Simple 1206
	if(!stage)
	posn(x,y,3.2,1.6,r,0.6,0.6)
	{
		cube([3.2,1.6,1]);
		translate([-0.5,-0.5,0])
		cube([3.2+1,1.6+1,0.5]); // Solder
	}
}

module smdrelay(stage,x,y,r=0)
{ // Solid state relay RS part 6839012
	if(!stage)
	posn(x,y,4.4,3.9,r,0.6,0.6)
	{
		cube([4.4,3.9,3]);
		translate([-1.5,0,0])
		cube([4.4+3,3.9,2]);	// Solder and tags
	}
}

module spox(stage,x,y,r=0,n=2,pcb=1.6,hidden=false)
{
	w=(n-1)*2.5+4.9;
	posn(x,y,w,7.9,r)
	{
		if(!stage)
		{
			pads(2.45,7.5,1.2,2.5,n,2.5);
			translate([0,0,-pcb-4.9])
			cube([w,4.9,4.9]);
			translate([0,0,-pcb-3.9])
    			cube([w,5.9,3.9]);
    			hull()
    			{
				translate([0,0,-pcb-0.5])
        			cube([w,7.9,0.5]);
				translate([0,0,-pcb-1])
        			cube([w,7.4,1]);
    			}
			translate([4.9/2-0.3,0,-pcb-2.38-0.3])
    			cube([w-4.9+0.6,6.6+0.3,2.38+0.3]);
			if(!hidden)
			{
				translate([0,-20,-pcb-4.9])
    				cube([w,20,4.9]);
			}
		}else if(!hidden)
		{ // Cut
			translate([0,-20,-pcb-2])
			hull()
			{
				translate([0,0,stage/2])
				cube([w,20,1]);
				translate([0,0,stage*20])
				cube([w,20,1]);
			}
		}
	}
}

module usbc(stage,x,y,r=0)
{
	posn(x,y,8.94,7.35,r)
	{
		if(!stage)
		{
			cube([8.94,7.35,3.26/2]);
			cube([8.94,8,0.5]);	// Solder
			hull()
			{
				translate([-1,1.88-1,0])
				cube([8.94+2,1.4+2,0.5]); // Solder
				translate([-0.4,1.88-0.4,0])
				cube([8.94+0.8,1.4+0.8,1.5]); // Solder
			}
			hull()
			{
				translate([-1,5.91-1,0])
				cube([8.94+2,1.7+2,0.5]); // Solder
				translate([-0.4,5.91-0.4,0])
				cube([8.94+0.8,1.7+0.8,1.5]); // Solder
			}
			// Posts
			for(px=[-0.155,8.495])
			{
				translate([px,1.88,-1])
				cube([0.9,1.4,1]);
				translate([px,5.91,-1])
				cube([0.9,1.7,1]);
			}
			for(px=[1.28+.3,7.06+0.3])
			translate([px,5.98+.3,-1])
			cylinder(d=0.6,h=1);
			// lead and body
			translate([3.26/2,-2,3.26/2])
			rotate([-90,0,0])
			{
				hull()
				{
					cylinder(d=3.26,h=2+7.35);
					translate([8.94-3.26,0,0])
					cylinder(d=3.26,h=2+7.35);
				}
				// Body of plug
				translate([0,0,-20])
				hull()
				{
					cylinder(d=7,h=20);
					translate([8.94-3.26,0,0])
					cylinder(d=7,h=20);
				}
			}
		}else{ // Cut
			translate([0,-20,3.26/2-0.5])
			hull()
			{
				translate([-1.5,0,stage/2])
				cube([8.94+3,20.49,1]);
				translate([-5,0,stage*20])
				cube([8.94+10,20.49,1]);
			}
		}
	}
}

module oled(stage,x=0,y=0,r=0,d=5,h=6)
{ // OLED module e.g. https://www.amazon.co.uk/gp/product/B07BDMG2DK
	// d / h are the pillars
	posn(x,y,45,37,r)
	{
		if(!stage)
		{
			pads(3,9.71,0.9,2.5,1,2.54,2,2.54);
			pads(3,9.71+3*2.54,0.9,2.5,1,2.54,2,2.54);
			translate([0,0,-h-1.6])
			mirror([0,0,1])
			{
				pads(1.637,9.71,0.9,2.5,1,2.54,2,2.54);
				pads(1.637,9.71+3*2.54,0.9,2.5,1,2.54,2,2.54);
			}
			translate([0,0,-h-1.6])
			cube([45,37,h+1.6]);
			for(px=[2.75,42.25])
            		for(py=[2.5,34.5])
            		translate([px,py,-h])
			{
            			if(d>5)cylinder(d=d,h=h); // Pillar
				translate([0,0,-1.6-2])
				cylinder(d=5,h=2); // Screws
			}
		        translate([5,0,-h-1.6-2])
        		cube([35,37,2]);
		}else if(stage==-1){ // cut
        		hull()
        		{ // Window for view
            			translate([6.5,1.5,-h-1.6-2-1])
            			cube([30,28,1]);
            			translate([6.5-5,2-5,-h-1.6-2-1-20])
            			cube([30+10,28+10,1]);
        		}
		}
	}
}
