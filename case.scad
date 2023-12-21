// Generate PCB casework

height=casebase+pcbthickness+casetop;
$fn=48;

module pyramid()
{ // A pyramid
 polyhedron(points=[[0,0,0],[-height,-height,height],[-height,height,height],[height,height,height],[height,-height,height]],faces=[[0,1,2],[0,2,3],[0,3,4],[0,4,1],[4,3,2,1]]);
}


module pcb_hulled(h=pcbthickness,r=0)
{ // PCB shape for case
	if(useredge)outline(h,r);
	else hull()outline(h,r);
}

module solid_case(d=0)
{ // The case wall
	hull()
        {
                translate([0,0,-casebase])pcb_hulled(height,casewall-edge);
                translate([0,0,edge-casebase])pcb_hulled(height-edge*2,casewall);
        }
}

module preview()
{
	pcb();
	color("#0f0")parts_top(part=true);
	color("#0f0")parts_bottom(part=true);
	color("#f00")parts_top(hole=true);
	color("#f00")parts_bottom(hole=true);
	color("#00f")parts_top(block=true);
	color("#00f")parts_bottom(block=true);
}

module top_half()
{
	translate([-casebase-1,-casewall-1,pcbthickness+0.01]) cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,height]);
}

module bottom_half()
{
	translate([-casebase-1,-casewall-1,pcbthickness-height-0.01]) cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,height]);
}

module top_body()
{
	difference()
	{
		intersection()
		{
			solid_case();
			pcb_hulled(height,0.01);
			top_half();
		}
		minkowski()
		{
			hull()parts_top(part=true);
			translate([0,0,margin-height])cylinder(r=margin,h=height,$fn=8);
		}
		parts_top(hole=true);
		parts_bottom(part=true);
	}
}

module top_edge()
{
	difference()
	{
		intersection()
		{
			solid_case();
			top_half();
		}
		pcb_hulled(height);
		minkowski()
		{
			parts_top(part=true,hole=true);
			sphere(r=margin,$fn=8);
		}
	}
}

module top()
{
	translate([casewall,casewall+pcblength,pcbthickness+casetop])rotate([180,0,0])
	{
		top_body();
		top_edge();
	}
}

module bottom_body()
{
	difference()
	{
		intersection()
		{
			solid_case();
			translate([0,0,-height])pcb_hulled(height,0.01);
			bottom_half();
		}
		minkowski()
		{
			hull()parts_bottom(part=true);
			translate([0,0,-margin])cylinder(r=margin,h=height,$fn=8);
		}
		parts_bottom(hole=true);
		parts_top(part=true);
	}
}

module bottom_edge()
{
        difference()
        {
                intersection()
                {
                        solid_case();
                        bottom_half();
                }
                translate([0,0,-height])pcb_hulled(height+pcbthickness+1);
                minkowski()
                {
                        parts_bottom(part=true,hole=true);
                        sphere(r=margin,$fn=8);
                }
        }
}

module bottom()
{
	translate([casewall,casewall,casebase])
	{
        	bottom_body();
        	bottom_edge();
	}
}



translate([spacing*2,0,0])preview();
