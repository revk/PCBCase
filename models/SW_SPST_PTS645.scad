if(!hulled&&!pushed)
{
	b(0,0,0,6,6,4); // Used as a tamper, so exact (i.e. !pushed)
	for(x=[-2,2])for(y=[-2,2])translate([x,y,0])cylinder(d=1,h=4.2);
}
b(0,0,0,9,6,1.1); // Legs

