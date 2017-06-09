#include "semshm.c"
#include "gd.h"
#include "gdfonts.h"
#include <time.h>

#include "cgilib.c"

#undef DEBUG

#define c_blank 245,245,245
#define c_light 194,194,194
#define c_dark  100,100,100
#define c_major 255,0,0
#define c_rp    0,235,12
#define c_tp    0,94,255
#define c_grid  0,0,0
#define c_rb    0,166,33
#define c_tb    255,0,255
#define c_x     239,159,79
#define c_dash  50,50,50

int i_blank,i_light,i_dark,i_grid,i_rp,i_tp,i_rb,i_tb,i_dash,i_x;

#define XSIZE  282
#define XZ     50
#define YSIZE  200
#define YZ     25

#define SEC    1
#define MIN    2
#define T12MIN 3
#define T2HOUR 4

char * toGMKdec(u32 t)
{
	static char buf[MAXLEN];
	if(t>=1000*1000*1000) 
		snprintf(buf,MAXLEN,"%.1fG",(float)(((float)t)/1000/1000/1000));
	else if(t>=1000*1000) 
		snprintf(buf,MAXLEN,"%.1fM",((float)t)/1000/1000);
	else if(t>=1000) 
		snprintf(buf,MAXLEN,"%.1fK",((float)t)/1000);
	else 
		snprintf(buf,MAXLEN,"%d",(int)t);
	return buf;
}


u32 maxtoMAXdec(u32 t){
	int got,i;
	u32 rate;
	if(t<=10) return 10;
	got=0;
	for(rate=1;;rate=rate*10) {
		for(i=2;i<=11;i++) 
			if( ((i-1)*rate<t) && (t<=i*rate)) {
				got=1;
				break;
			}
		if(got) break;
	}
	t=i*rate;
	if(t<10) t=10;
	return t;
}

u32 maxtoMAX(u32 t){
	int got,i;
	u32 rate;
	if(t<=10) return 10;
	got=0;
	for(rate=1;;rate=rate*10) {
		for(i=2;i<=11;i++) 
			if( ((i-1)*rate<t) && (t<=i*rate)) {
				got=1;
				break;
			}
		if(got) break;
	}
	t=i*rate;
	if(t<10) t=10;
	return t;
}


gdImagePtr graph;

/* draw blank image, with X,Y */
void drawblankimage(u32 MAXB, u32 MAXP) {
	int i;
        graph = gdImageCreate(XSIZE, YSIZE);
	i_blank = gdImageColorAllocate(graph,c_blank);
 	i_light = gdImageColorAllocate(graph,c_light);
	i_dark = gdImageColorAllocate(graph,c_dark);
	i_grid = gdImageColorAllocate(graph,c_grid);
	i_dash = gdImageColorAllocate(graph,c_dash);
	i_rp = gdImageColorAllocate(graph,c_rp);
	i_tp = gdImageColorAllocate(graph,c_tp);
	i_rb = gdImageColorAllocate(graph,c_rb);
	i_tb = gdImageColorAllocate(graph,c_tb);
	i_x = gdImageColorAllocate(graph,c_x);

   	gdImageLine(graph,0,0,XSIZE-1,0,i_light);
    	gdImageLine(graph,1,1,XSIZE-2,1,i_light);
    	gdImageLine(graph,0,0,0,YSIZE-1,i_light);
    	gdImageLine(graph,1,1,1,YSIZE-2,i_light);
    	gdImageLine(graph,XSIZE-1,0,XSIZE-1,YSIZE-1,i_dark);
    	gdImageLine(graph,0,YSIZE-1,XSIZE-1,YSIZE-1,i_dark);
    	gdImageLine(graph,XSIZE-2,1,XSIZE-2,YSIZE-2,i_dark);
    	gdImageLine(graph,1,YSIZE-2,XSIZE-2,YSIZE-2,i_dark);
	gdImageString(graph,gdFontSmall,9,2,"   BPS",i_grid);
	gdImageString(graph,gdFontSmall,XZ+180+9,2,"Pkt/s",i_grid);
	
	for(i=0;i<5;i++) {
		char *p;
		p = toGMKdec(MAXP*i/4);
		gdImageString(graph,gdFontSmall,XSIZE-10-gdFontSmall->w*strlen(p),YZ+(4-i)*25-gdFontSmall->h/2,p,i_grid);
		p = toGMKdec(MAXB*i/4);
		gdImageString(graph,gdFontSmall,XZ-3-gdFontSmall->w*strlen(p),YZ+(4-i)*25-gdFontSmall->h/2,p,i_grid);
	}
}

unsigned long int mintp,maxtp,avgtp;
unsigned long int minrp,maxrp,avgrp;
unsigned long int mintb,maxtb,avgtb;
unsigned long int minrb,maxrb,avgrb;

unsigned long int maxp,maxb,MAXP,MAXB;

void dostatis(u32 *d, int n, u32 *min, u32 *max, u32 *avg){
	float sum;
	int i;
	if(n<=0) {
		*min=*max=*avg=0;
		return;
	}
	*min=*max=sum=d[0];
	for(i=1;i<n;i++) {
		if(d[i]<*min) *min=d[i];
		if(d[i]>*max) *max=d[i];
		sum+=d[i];
	}
	*avg=sum/n;
	return;
}

void drawcontent(u32 *rp, u32 *tp, u32 *rb, u32 *tb, u32 start)
{	int i;
	char buf[MAXLEN];
	u32 oldstart;
	oldstart=start;
	maxp=maxb=0;
	dostatis(rp,180, &minrp, &maxrp, &avgrp);
	dostatis(tp,180, &mintp, &maxtp, &avgtp);
	dostatis(rb,180, &minrb, &maxrb, &avgrb);
	dostatis(tb,180, &mintb, &maxtb, &avgtb);

	maxp=maxrp>maxtp?maxrp:maxtp;
	maxb=maxrb>maxtb?maxrb:maxtb;

	MAXP=maxtoMAX(maxp);
	MAXB=maxtoMAX(maxb);

	drawblankimage(MAXB,MAXP); 


	gdImageSetPixel(graph,XZ,YZ+99-100*rb[start%180]/MAXB,i_rb);
	gdImageSetPixel(graph,XZ,YZ+99-100*rp[start%180]/MAXP,i_rp);
	gdImageSetPixel(graph,XZ,YZ+99-100*tb[start%180]/MAXB,i_tb);
	gdImageSetPixel(graph,XZ,YZ+99-100*tp[start%180]/MAXP,i_tp);
	start++;

	for(i=1;i<180;i++,start++) {

#ifdef DEBUG
	printf("tp i=%d start=%d %d\n",i,start,100*tp[start%180]/MAXP);
#endif

		gdImageLine(graph,XZ+i,YZ+99-100.0*rb[start%180]/MAXB,XZ+i,YZ+99,i_rb);
		gdImageLine(graph,XZ+i,YZ+99-100.0*rp[start%180]/MAXP,XZ+i,YZ+99-100.0*rp[(start-1)%180]/MAXP,i_rp);
		gdImageLine(graph,XZ+i,YZ+99-100.0*tb[start%180]/MAXB,XZ+i,YZ+99-100.0*tb[(start-1)%180]/MAXB,i_tb);
		gdImageLine(graph,XZ+i,YZ+99-100.0*tp[start%180]/MAXP,XZ+i,YZ+99-100.0*tp[(start-1)%180]/MAXP,i_tp);
	} 
	
	gdImageLine(graph,XZ,YZ+99,XZ+179,YZ+99,i_x);

	snprintf(buf,MAXLEN,"Input  Pkt/s:max %6s",toGMKdec(maxrp));
	snprintf(buf+strlen(buf),MAXLEN," avg %6s",toGMKdec(avgrp));
	snprintf(buf+strlen(buf),MAXLEN," last %6s",toGMKdec(rp[(oldstart+179)%180]));
	gdImageString(graph,gdFontSmall,3,YZ+110,buf,i_rp);
	
	snprintf(buf,MAXLEN,"Output Pkt/s:max %6s",toGMKdec(maxtp));
	snprintf(buf+strlen(buf),MAXLEN," avg %6s",toGMKdec(avgtp));
	snprintf(buf+strlen(buf),MAXLEN," last %6s",toGMKdec(tp[(oldstart+179)%180]));
	gdImageString(graph,gdFontSmall,3,YZ+110+(gdFontSmall->h+2),buf,i_tp);

	snprintf(buf,MAXLEN,"Input  BPS  :max %6s",toGMKdec(maxrb));
	snprintf(buf+strlen(buf),MAXLEN," avg %6s",toGMKdec(avgrb));
	snprintf(buf+strlen(buf),MAXLEN," last %6s",toGMKdec(rb[(oldstart+179)%180]));
	gdImageString(graph,gdFontSmall,3,YZ+110+2*(gdFontSmall->h+2),buf,i_rb);

	snprintf(buf,MAXLEN,"Output BPS  :max %6s",toGMKdec(maxtb));
	snprintf(buf+strlen(buf),MAXLEN," avg %6s",toGMKdec(avgtb));
	snprintf(buf+strlen(buf),MAXLEN," last %6s",toGMKdec(tb[(oldstart+179)%180]));
	gdImageString(graph,gdFontSmall,3,YZ+110+3*(gdFontSmall->h+2),buf,i_tb);
}

void drawimage(char* devname, int type)
{
	RRD rrd,*prrd;
	char buf[MAXLEN];
	unsigned long int start,i,n;
	int lastprt=0;
	int found=0;
	
	memcpy(&rrd,shm,sizeof(RRD));
	for(i=0;i<rrd.totalint;i++) {
		prrd = ((RRD*)shm)+i;
		if(strcmp(prrd->name,devname)==0) {
			found = 1;
			break;
		}
	}
	if(found==0) return;
	memcpy(&rrd,shm+i*sizeof(RRD),sizeof(RRD));
	switch(type) {
		struct tm *ptm;
		case MIN:
			start = rrd.lastmin-60*179; 
			ptm=localtime(&start);
			n=ptm->tm_hour*60+ptm->tm_min;
			drawcontent(rrd.mindatarp,rrd.mindatatp,rrd.mindatarb,rrd.mindatatb,n);
			break;
		case T12MIN:
			start = rrd.lastt12min-720*179;
			n=start/720;
			drawcontent(rrd.t12mindatarp,rrd.t12mindatatp,rrd.t12mindatarb,rrd.t12mindatatb,n);
			break;
		case T2HOUR:
			start = rrd.lastt2hour-7200*179;
			n=start/7200;
			drawcontent(rrd.t2hourdatarp,rrd.t2hourdatatp,rrd.t2hourdatarb,rrd.t2hourdatatb,n);
			break;
		case SEC:
		default:
			start = rrd.lastsec - 179; 
			ptm=localtime(&start);
			n=ptm->tm_min*60+ptm->tm_sec;
			type=SEC;
			drawcontent(rrd.secdatarp,rrd.secdatatp,rrd.secdatarb,rrd.secdatatb,n);
	}

	
	for(i=0;i<180;i++) {
		struct tm *ptm;
		ptm=localtime(&start);

		switch(type) {
		case SEC:
			if(ptm->tm_sec==0) {			
				snprintf(buf,MAXLEN,"%dm",ptm->tm_min);
				gdImageString(graph,gdFontSmall,XZ+(i%180)-strlen(buf)*gdFontSmall->w/2,YZ+100,buf,i_grid);
			}
			if(ptm->tm_sec%30==0)
				gdImageDashedLine(graph,XZ+(i%180),YZ,XZ+(i%180),YZ+99,i_dash);
			start++;
			break;
		case MIN:
			if(ptm->tm_min==0) {
				snprintf(buf,MAXLEN,"%dh",ptm->tm_hour);
				gdImageString(graph,gdFontSmall,XZ+(i%180)-strlen(buf)*gdFontSmall->w/2,YZ+100,buf,i_grid);
			}
			if((ptm->tm_min%30)==0) 
				gdImageDashedLine(graph,XZ+(i%180),YZ,XZ+(i%180),YZ+99,i_dash);
			start+=60;
			break;
		case T12MIN:
			if( (ptm->tm_hour==0) && (lastprt!=ptm->tm_mday) ) {
				lastprt=ptm->tm_mday;
				snprintf(buf,MAXLEN,"%dd",ptm->tm_mday);
				gdImageString(graph,gdFontSmall,XZ+(i%180)-strlen(buf)*gdFontSmall->w/2,YZ+100,buf,i_grid);
			}
			if(  (ptm->tm_hour*60+ptm->tm_min)%360==0 )  {
				gdImageDashedLine(graph,XZ+(i%180),YZ,XZ+(i%180),YZ+99,i_dash);
			}
			start+=720;
			break;

		case T2HOUR:
			if( (ptm->tm_mday*24+ptm->tm_hour)%72==0) {
				lastprt=ptm->tm_mday;
				snprintf(buf,MAXLEN,"%dd",ptm->tm_mday);
				gdImageString(graph,gdFontSmall,XZ+(i%180)-strlen(buf)*gdFontSmall->w/2,YZ+100,buf,i_grid);
				gdImageDashedLine(graph,XZ+(i%180),YZ,XZ+(i%180),YZ+99,i_dash);
			}
			start+=7200;
			break;
	}
	}

#ifndef DEBUG
        gdImagePng(graph, stdout);
#endif
	fflush(NULL);
}


void trafficdisp()
{	char *p;
	int type,i;
	char *devname;
	RRD rrd,*prrd;
	if(pv_init()<0) exit(-1);
	if(getshm(0)<0) exit(-1);
	p=GetValue("dev");
	if(p) {
		printf("Content-type: image/png\r\n\r\n");	
		fflush(NULL);
		devname=p;
		p=GetValue("type");
		if(p==(char*)0) type=0;
		else type=atoi(p);
		drawimage(devname,type);
		exit(0);
	}
	printf("Content-type: text/html\r\n\r\n");	
	printf("<HTML>");
	printf("<HEAD><TITLE>流量统计图</TITLE><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /> <META HTTP-EQUIV=\"Refresh\" CONTENT=\"60\"> <META HTTP-EQUIV=\"Cache-Control\" content=\"no-cache\"> <META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n");
	printf("</HEAD> <STYLE type=text/css>BODY { FONT-FAMILY: verdana, sans-serif, helvetica; MARGIN-LEFT: 1em; MARGIN-TOP: 2em } TH { BACKGROUND: #000099; COLOR: white; FONT-FAMILY: verdana, sans-serif, helvetica; FONT-SIZE: 9pt; FONT-WEIGHT: bold; TEXT-ALIGN: center } TD { FONT-FAMILY: verdana, sans-serif, helvetica; FONT-SIZE: 9pt } </STYLE> <H3>流量统计图</H3> <BODY bgColor=#ffffff> \n");

	printf("<form action=/cgi-bin/traffic method=get>");
	printf("<input name=traffic value=1 type=hidden>");


	memcpy(&rrd,shm,sizeof(RRD));
	for(i=0;i<rrd.totalint;i++) {
		prrd = ((RRD*)shm)+i;
		printf("<input name=devs type=checkbox value=%s>%s</input>&nbsp; ",prrd->name,prrd->name);
	}
	
	printf("<input type=submit value=刷新>");
	printf("</form>\n");

	printf("<table bgcolor=#cccccc border cellpadding=2 cellspacing=0> \n");
	printf("<tr><th>图形含义</th>");

	p=GetValue("devs");
	if(!p) printf("<th>eth0</th>");  
	else while(p && *p) {
		printf("<th>%s</th>",p); p=GetNextValue("devs");
	}	
	printf("</tr>\n");
	
	printf("<tr><td nowrap>3分钟</td>");
	p=GetValue("devs");
	if(!p) printf("<td><img src=/cgi-bin/traffic?dev=eth0&type=1></td>");
	else while(p && *p) {
		printf("<td><img src=/cgi-bin/traffic?dev=%s&type=1></td>",p); p=GetNextValue("devs");
	}	
	printf("</tr>\n");

	printf("<tr><td nowrap>3小时</td>");
	p=GetValue("devs");
	if(!p) printf("<td><img src=/cgi-bin/traffic?dev=eth0&type=2></td>");
	else while(p && *p) {
		printf("<td><img src=/cgi-bin/traffic?dev=%s&type=2></td>",p); p=GetNextValue("devs");
	}	
	printf("</tr>\n");


	printf("<tr><td nowrap>1.5天</td>");
	p=GetValue("devs");
	if(!p) printf("<td><img src=/cgi-bin/traffic?dev=eth0&type=3></td>");
	else while(p && *p) {
		printf("<td><img src=/cgi-bin/traffic?dev=%s&type=3></td>",p); p=GetNextValue("devs");
	}	
	printf("</tr>\n");


	printf("<tr><td nowrap>15天</td>");
	p=GetValue("devs");
	if(!p) printf("<td><img src=/cgi-bin/traffic?dev=eth0&type=4></td>");
	else while(p && *p) {
		printf("<td><img src=/cgi-bin/traffic?dev=%s&type=4></td>",p); p=GetNextValue("devs");
	}	
	printf("</tr></table>\n");
	printf("%s","<p>图形含义：<p> 3分钟的图中，每个点分别表示1秒钟的间隔内收、发数据包的个数以及比特。<br>3小时的图中，每个点分别表示1分钟的间隔内，每秒钟平均收、发数据包的个数以及比特。<br> 1.5天的图中，每个点分别表示12分钟的间隔内，每秒钟平均收、发数据包的个数以及比特。<br> 15天的图中，每个点分别表示2小时的间隔内，每秒钟平均收、发数据包的个数以及比特。<br>"); 
}

CGImain() {
      trafficdisp();
      exit(0);
}
