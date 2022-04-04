$fn=90;
//include <threads.scad>;
include </home/claudea2/Dropbox/3D printer/openscad_primitives-master/fillet_cube.scad>;
// rev 4 2 slots, 3.5mm instead of 5mm long
overlap =5;
len2 = 10;
len3= 5.16;
len4= 8.9;
diam1= (3.7*25.4)+2.4;
diam2= 5*25.4;  //  was 6*25.4;
ventholesdiam =3;
diam3= 13.6;
diam4= 9.6;
glassdiam=200;
fandiam=120;
largewall=3.2;
smallwall = 1.6;
 baseholediam= 6;
totallen = 104.6;
 pindiam=1.65;  // pogo pin holder diampindiam 1.3 bump 1.5
  wirechannel=1.6;
pcbwidth=40.5;
offx=65;
offy=93.6;
coin1x= -16.9;
coin1y = -65.47;
coin2x= 49;
coin2y = 0;
coin3x= 49;
coin3y = -58;
coin4x=  0;
coin4y= -58;
adcx=56.02  ;
adcy= 65.03;
pin=2.75; //  mounting hole in pcb
pogo1x=-1.27;  //2.54mmx2
pogo1y=4.87;
pogo2x=5.9;  // 3.5mm
pogo2y=7.53;
pogo3x=5.9;
pogo3y=27.53;
pogo4x=6.1;
pogo4y=48.28;
pogo5x=33.2; // 2.54mm
pogo5y=44.93;
screwhole= 3.0;
// pogo IDI   R-1-SC/RSS-75-SC
// receptacle  S-1-E-3.8-G


pogoheight = 6.59 +2.5;
pcbsspacer = 8.64+4.57-1.78-5; // 11.43
pcbheight=178;
pcbthick= 1.6;
//height above PCB
height4= pogoheight+pcbthick;
// for test add 10mm    
len1 = 10 + totallen - len2 -len3 -len4 +overlap;
 fit=0.6; 
mountingspherediam=3;
// measuring tool
//translate([52.5,0,-26]) cube([1,105,1],center=true);
//show = 0;
//show="PCBClips"; // ,   "    " ,       "    ";// "fan";
//show="mount";  // hanging mount
//show="topbox";
//show = "fan";
//show = "PCBBaseclip";

// uncomment to see

//selector ="main";
//selector ="base";

//selector ="push";
//selector = "bolt";
//selector = "tb";
//selector ="break";
selector = "case";
/*********************************************************
*
*        Pogo test jig for ADC rev 2
*
*
**************************************************************/



//color( "Aqua", 1.0 )translate([offx+coin1x,coin1y-offy,pcbsspacer])  import("adc.stl", convexity=3);
module mainpin(){
 // main body above z=0
difference(){
// base cube    
translate([-8,-8,0])  cube([67,75,5]) ;
    
 // larger cut-out for better fit 
translate([-6,-6,-0.05])  cube([adcx+6,adcy+6,8.1]) ;
}
// round corner for better printing
  translate([-6,-6,-7.9])   cylinder(d=8,h=13-0.1);
translate([adcx,-6,-7.9])   cylinder(d=8,h=13-0.1);
translate([-6,+adcy,-7.9])   cylinder(d=8,h=13-0.1);
translate([adcx,+adcy,-7.9])   cylinder(d=8,h=13-0.1);
// space for align pins pcb at correct height
translate([0,0,-0.1]) alignpins(4,4,pcbsspacer);
// Align pins
translate([0,0,pcbsspacer-0.1]) alignpins(2.4,2,4.5);
// base pogo receptacle fit into base
difference(){
translate([-8,-8,-7.9])  cube([67,75,8]) ;
 translate([0,0,-8])    pinholes(10);
}
// align pins
module alignpins(dd1,dd2,hh){
translate([0,0,0])   cylinder(d1=dd1,d2=dd2,h=hh);
translate([coin2x,coin2y,0])  cylinder(d1=dd1,d2=dd2,h=hh);
translate([coin3x,-coin3y,0])  cylinder(d1=dd1,d2=dd2,h=hh);
translate([coin4x,-coin4y,0])  cylinder(d1=dd1,d2=dd2,h=hh);
}
module pinholes(hh){
for(yi =[pogo1y:5.08:pogo1y+5.08]) translate([pogo1x,yi,0]) cylinder(d=pindiam,h=hh);
for(xi =[pogo5x:2.54:pogo5x+(5*2.54)]) translate([xi,pogo5y,0]) cylinder(d=pindiam,h=hh);
    
for(xi =[pogo2x:3.5:pogo2x+(4*3.5)]) translate([xi,pogo2y,0]) cylinder(d=pindiam,h=hh);
for(xi =[pogo3x:3.5:pogo3x+(4*3.5)]) translate([xi,pogo3y,0]) cylinder(d=pindiam,h=hh);
for(xi =[pogo4x:3.5:pogo4x+(4*3.5)]) translate([xi,pogo4y,0]) cylinder(d=pindiam,h=hh);
}
 
}
module base2(){
// bottom base
translate([0,0,15])difference(){
translate([-8-4,-8-4,-40])  cube([67+8,75+8,25]) ;
translate([-8,-8,-40+4])  cube([67,75,30]) ;
translate([-6-33/2,-6-34/2,-41])   cube([33,34,30]);
translate([adcx-33/2,-6-34/2,-41])   cube([33,34,30]);
translate([-6-33/2,+adcy-34/2,-41])   cube([33,34,30]);
translate([adcx-33/2,+adcy-34/2,-41])   cube([33,34,30]);
}
// holder for pushdown
translate([-55,18,-25])  cube([44,25,10]) ;
difference(){
translate([-55,30,-25])  cube([12,12,35]) ;
translate([-46,50,6]) rotate([90,0,0])   cylinder(d=screwhole+0.3, h=50);  
translate([-52,50,6]) rotate([90,0,0])   cylinder(d=screwhole+0.3, h=50);      
}
// tye point spring
difference(){
translate([-25,24,-25])  cube([8,6,15]) ;
translate([-21,50,-12.5]) rotate([90,0,0])   cylinder(d=screwhole, h=50);  
}     
}


if (selector =="main"){
difference(){
      mainpin();
     translate([26,79,-4]) rotate([90,0,0])   cylinder(d=screwhole, h=100);
     translate([-26,-6+79/2,-4]) rotate([90,0,90])   cylinder(d=screwhole, h=100);
  
}
}

if (selector =="base"){
difference(){
      base2();
     translate([26,79,-4]) rotate([90,0,0])   cylinder(d=screwhole+0.5, h=100);
     translate([-26,-6+79/2,-4]) rotate([90,0,90])   cylinder(d=screwhole+0.5, h=100);
  
}
}
if (selector =="push"){

pushdown();
}
// push down
module pushdown(){
 translate([-11,0,0])difference(){
 translate([0,0,0]) rotate([0,0,0])   cylinder(d=76, h=10);
 translate([0,0,-0.1]) rotate([0,0,0])   cylinder(d=61, h=11);
  translate([0,-25,5.5])  cube([100,50,12],center=true);   
    translate([-34,4,-1]) rotate([0,0,0])   cylinder(d=screwhole+0.3, h=20);
    
}
 // lever
    translate([-20,8,0]) rotate([0,0,-2])   cube([150,25,10]);  
}


// bolt holder
if (selector =="bolt"){
    bolt();
}

/******************************************************
*
*   replace a centered square cube with a rounded cube
*
*
************************************************************/
module roundedcube(xx,yy,zz,rr){
    translate([(-xx+(2*rr))/2,(-yy+(2*rr))/2,-zz/2]) minkowski()
{
  cube([xx-(2*rr),yy-(2*rr),zz-1]);
  cylinder(r=rr,h=1);
}
}

if (selector =="break"){
    difference(){
         cube([30,20,12],center=true); 
        translate([0,0,6-2.4])   cube([54,2,5],center=true); 
    }
    translate([0,0,-50-5.5])     cylinder(d=20, h=50);
}


module bolt(){
    
 /*  translate([-(58-2)/2,-(28-2)/2,-5]) minkowski()
{
  cube([60-4,30-4,10-1]);
  cylinder(r=2,h=1);
}
*/
    difference(){
         roundedcube(60,30,10,2); 
        translate([0,0,-0.1])   cube([10.5,10.5,12],center=true);  
        translate([0,0,-0.1])     cylinder(d=22, h=6.5);
        translate([30-6,15-6,-7])     cylinder(d=4, h=14);
        translate([30-6,-15+6,-7])     cylinder(d=4, h=14);
        translate([-30+6,15-6,-7])     cylinder(d=4, h=14);
        translate([-30+6,-15+6,-7])     cylinder(d=4, h=14); 
    }
    
    translate([0,40,0]) difference(){
         roundedcube(40,30,10,2);  
         translate([10,0,-6.1])cylinder(d=9.7, h=12);
    }
  
}
if (selector=="case"){
    casey=66;
    casex=57;
  translate([casex/2-4,casey/2-4,2])  difference(){
         roundedcube(casex+4,casey+4,7,3.5); 
        translate([0,0,2])  roundedcube(casex,casey,7,3.5); 
    } 
 
// space for align pins pcb at correct height
translate([0,0,-0.1]) alignpins2(4,4,pcbsspacer-1);
// Align pins
translate([0,0,pcbsspacer-1.15]) alignpins2(3.2,2.6,3);
    // bumps
    /*
        translate([0,0,pcbsspacer+1])   sphere(d=2.7);
translate([coin2x,coin2y,pcbsspacer+1])  sphere(d=2.7);
translate([coin3x,-coin3y,pcbsspacer+1])  sphere(d=2.7);
translate([coin4x,-coin4y,pcbsspacer+1])  sphere(d=2.7);
    */
// mounting holes
   translate([3,-8,-1.5]) holeD(3.3,7,3.3+1.8+1.8);
   translate([3,+casey,-1.5]) holeD(3.3,7,3.3+1.8+1.8);
translate([casex-8-3,-8,-1.5]) holeD(3.3,7,3.3+1.8+1.8);
   translate([casex-8-3,+casey,-1.5]) holeD(3.3,7,3.3+1.8+1.8);
// text
    
 translate([casex-2.5,12,-0.9])   rotate([90,0,90]) linear_extrude(1,convexity=1)text(size=5.5,"SEENOV");
}
// align pins
module  alignpins2(dd1,dd2,hh){
translate([0,0,0])   cylinder(d1=dd1,d2=dd2,h=hh);
translate([coin2x,coin2y,0])  cylinder(d1=dd1,d2=dd2,h=hh);
translate([coin3x,-coin3y,0])  cylinder(d1=dd1,d2=dd2,h=hh);
translate([coin4x,-coin4y,0])  cylinder(d1=dd1,d2=dd2,h=hh);

}
/***************************************************************
*            holder for terminal blocks
*
****************************************************************/
if (selector =="tb"){
     difference(){
         roundedcube(17.5+10,10+6,4,1); 
         translate([0,0,-0.1])  cube([17.5,10,6],center=true); 
         translate([-2.5+(17.5+10)/2,0,-6])     cylinder(d=3.3, h=12);
         translate([-(-2.5+(17.5+10)/2),0,-6])     cylinder(d=3.3, h=12);
     }
     for (i = [0,7,-7]) { 
         translate([i,5,2-(1.64/2)]) sphere(d=0.8); 
         translate([i,-5,2-(1.64/2)]) sphere(d=0.8); 
         } 
         translate([-2.5+(17.5+10)/2,0,1.9])  holeD(3.3,8,3.3+1.7);
         translate([-(-2.5+(17.5+10)/2),0,1.9])  holeD(3.3,8,3.3+1.7);
}


//    *******************************************************
// old design no longer used
//
//
//  *********************************************************
 module mainBody(){

  
//translate([-7.29,-8.31,1.5+8])color( "Aqua", 1.0 ) import("FH1.stl", convexity=3);
 
//***************************
// rectangle holder
//***************************
yy=55+0.4;
xx=64.4+0.4;
thick1=4.7;
standoffheight=pogoheight;

union(){
translate([87.455,-76.9,height4/2])difference(){
        cube([xx,yy,height4], center = true); 
        translate ([0,0,-0.1])cube([xx-thick1,yy-thick1,13], center = true); 
}
//****************************
//     standoff for pcb
//*****************************


#translate ([58,-68.5,0])cylinder (d=4, h=pogoheight);
translate ([117,-68.5,0])cylinder (d=4, h=pogoheight);
translate ([85,-52,0])cylinder (d=4, h=pogoheight);
translate ([87,-102,0])cylinder (d=4, h=pogoheight);
translate ([117,-95,0])cylinder (d=4, h=pogoheight);
translate ([68,-102,0])cylinder (d=4, h=pogoheight);

//****************************
//     mounting
//*****************************
translate ([52.98,-68.5,0])hole(2.2,height4);
translate ([52.98+69.02,-66.48,0])hole(2.2,height4);
translate ([71.5,-47.99,0])hole(2.2,height4);
translate ([75,-107.5,pogoheight/2])cube([5.6,7,pogoheight], center = true);
translate ([75,-112.5,0])hole(2.2,height4);
}

}
//****************************
//     locking
//*****************************
module locking(){
translate ([0,0,0])hole(2.2,2.4);
translate ([6+1.1,0,2.4/2])cube([12,5,2.4], center = true);
}

//*************************************
//     cylinder with center hole
//*************************************

module hole(dd,hh){
  difference(){
          cylinder(d=dd+4, h= hh);
    translate ([0,0,-0.1])cylinder (d=dd, h=hh+1);
}
}

module holeD(dd,hh,OO){
  difference(){
          cylinder(d=OO, h= hh);
    translate ([0,0,-0.1])cylinder (d=dd, h=hh+1);
}
}

module PCBsupport(){
        //   U shaped support
       // %translate([-145,124.1,0])  import("FH.stl", convexity=3);
        translate([0,0,0])  cube([190+24,12,20], center = true); 
        translate([(190+12)/2,(105+12)/2,0])  cube([12,105,20], center = true); 
        
        translate([0,105+12,0])  cube([190+24,12,20], center = true); 
        // 6 screw holes for pcb screw in
        translate([4,10.1,-10])    holeD(2.95,20,10);
        translate([-91,10.1,-10])    holeD(2.95,20,10);
        translate([89,10.1,-10])    holeD(2.95,20,10);
        translate([0,97,0]) union(){
        translate([4,10.1,-10])    holeD(2.95,20,10);
        translate([-91,10.1,-10])    holeD(2.95,20,10);
        translate([89,10.1,-10])    holeD(2.95,20,10);
        }
        // 4 mounting holes full unit
        
        translate([0,118,0]) union(){
        translate([70,10.1,-10])    holeD(3.9,7,15);
         translate([-70,10.1,-10])    holeD(3.9,7,15);        
        }
         translate([0,-22,0]) union(){
        translate([70,10.1,-10])    holeD(3.9,7,15);
         translate([-70,10.1,-10])    holeD(3.9,7,15);        
        }
        
}

