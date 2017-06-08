#include "semshm.c"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

time_t curt;
struct tm *curtm;

void AddCountT2Hour(RRD *rrd, u32 rp, u32 rb, u32 tp, u32 tb){ /*每2天一次 */
	int n;
	n=(curt/7200)%180;
#ifdef DEBUG	
printf("T2Hour: %3d: %ul %ul %ul %ul\n",n,rp,rb,tp,tb);
fflush(NULL);
#endif
	rrd->t2hourdatarp[n]=rp;
	rrd->t2hourdatarb[n]=rb;
	rrd->t2hourdatatp[n]=tp;
	rrd->t2hourdatatb[n]=tb;
	rrd->lastt2hour=curt;
}

void AddCountT12Min(RRD *rrd, u32 rp, u32 rb, u32 tp, u32 tb){ /*每12分钟一次 */
	int n;
	n=(curt/720)%180;
	if( (curtm->tm_hour%2==0) && (curtm->tm_min==0) ) { /* 每2小时 */
		float sumrp,sumrb,sumtp,sumtb;
		int i;
		int start,end;
		sumrp=sumrb=sumtp=sumtb=0;
		start = (((n/10)-1)*10+180)%180; 
		end=start+9;
		for(i=start;i<=end;i++) {
			sumrp+=rrd->t12mindatarp[i];
			sumrb+=rrd->t12mindatarb[i];
			sumtp+=rrd->t12mindatatp[i];
			sumtb+=rrd->t12mindatatb[i];
		}
		sumrp/=10;	
		sumrb/=10;	
		sumtp/=10;	
		sumtb/=10;	
		AddCountT2Hour(rrd,sumrp,sumrb,sumtp,sumtb);
	}
#ifdef DEBUG	
printf("12Min: %3d: %ul %ul %ul %ul\n",n,rp,rb,tp,tb);
fflush(NULL);
#endif
	rrd->t12mindatarp[n]=rp;
	rrd->t12mindatarb[n]=rb;
	rrd->t12mindatatp[n]=tp;
	rrd->t12mindatatb[n]=tb;
	rrd->lastt12min=curt;
}

void AddCountMin(RRD *rrd, u32 rp, u32 rb, u32 tp, u32 tb){ /*每分钟一次 */
	int index;
	index=(curtm->tm_hour*60+curtm->tm_min)%180;
	if(index%12==0) {
		float sumrp,sumrb,sumtp,sumtb;
		int i;
		int start,end;
		sumrp=sumrb=sumtp=sumtb=0;
		if(index==0) { start = 168; end=179; }
		else { start = ((index/12)-1)*12; end=start+11;} 
		for(i=start;i<=end;i++) {
			sumrp+=rrd->mindatarp[i];
			sumrb+=rrd->mindatarb[i];
			sumtp+=rrd->mindatatp[i];
			sumtb+=rrd->mindatatb[i];
		}
		sumrp/=12;	
		sumrb/=12;	
		sumtp/=12;	
		sumtb/=12;	
		AddCountT12Min(rrd,sumrp,sumrb,sumtp,sumtb);
	}
#ifdef DEBUG	
printf("Min: %3d: %ul %ul %ul %ul\n",index,rp,rb,tp,tb);
fflush(NULL);
#endif
	rrd->mindatarp[index]=rp;
	rrd->mindatarb[index]=rb;
	rrd->mindatatp[index]=tp;
	rrd->mindatatb[index]=tb;
	rrd->lastmin=curt;
}

void AddCount(char*devname, u32 rp, u32 rb, u32 tp, u32 tb){ /* 每秒钟一次 */
	u32 index, lastsec;
	u32 rp1,rb1,tp1,tb1;
	int totalint,found,i;
	RRD *rrd;

#ifdef DEBUG
	printf("add %s %ld %ld\n",devname,rp,rb);
#endif	
	p_oper();  /* get shm */
	curt=time(NULL);
	curtm=localtime(&curt);
	rrd=((RRD *)shm);
	totalint=rrd->totalint;
	found = 0;
	for(i=0;i<totalint;i++) {
		rrd=((RRD*)shm)+i;
		if(strcmp(rrd->name,devname)==0) {
			found = 1;
			break;
		}
	}
	if(found ==0) { /* not found */ 
		if(totalint>=MAXINTS) return;
		rrd=((RRD *)shm);
		rrd->totalint++;
		rrd=((RRD *)shm)+totalint;
		strncpy(rrd->name,devname,MAXLEN);
		i=totalint;
		found = 1;
	}
	rrd=((RRD *)shm)+i;
	
	lastsec = rrd->lastsec;
	if(lastsec!=0) {
		if(rrd->lastrp>rp) rp1=rp; else rp1=rp- rrd->lastrp;
		if(rrd->lastrb>rb) rb1=rb; else rb1=rb- rrd->lastrb;
		if(rrd->lasttp>tp) tp1=tp; else tp1=tp- rrd->lasttp;
		if(rrd->lasttb>tb) tb1=tb; else tb1=tb- rrd->lasttb;
		rb1=rb1*8;
		tb1=tb1*8;
	} else {
		rrd->lastmin=curt-curtm->tm_sec;
		rrd->lastt12min=curt-curtm->tm_min*60-curtm->tm_sec;
		rrd->lastt2hour=curt-(curtm->tm_hour%2)*3600-curtm->tm_min*60-curtm->tm_sec;
		rp1=rb1=tp1=tb1=0;
	}
	rrd->lastrp=rp;	
	rrd->lastrb=rb;	
	rrd->lasttp=tp;	
	rrd->lasttb=tb;	
	
	index=(curtm->tm_min*60+curtm->tm_sec)%180;
	if( index%60==0) {
		float sumrp,sumrb,sumtp,sumtb;
		int i;
		int start,end;
		sumrp=sumrb=sumtp=sumtb=0;
		if(index==0) { start = 120; end=179; }
		else { start = ((index/60)-1)*60; end=start+59;} 
		for(i=start;i<=end;i++) {
			sumrp+=rrd->secdatarp[i];
			sumrb+=rrd->secdatarb[i];
			sumtp+=rrd->secdatatp[i];
			sumtb+=rrd->secdatatb[i];
		}
		sumrp/=60;	
		sumrb/=60;	
		sumtp/=60;	
		sumtb/=60;	
		AddCountMin(rrd,sumrp,sumrb,sumtp,sumtb);
	}

#ifdef DEBUG
printf("Sec: %3d: %ul %ul %ul %ul\n",index,rp1,rb1,tp1,tb1);
fflush(NULL);
#endif
	rrd->secdatarp[index]=rp1;
	rrd->secdatarb[index]=rb1;
	rrd->secdatatp[index]=tp1;
	rrd->secdatatb[index]=tb1;
	rrd->lastsec=curt;
	v_oper();  /* get shm */
}

void dostatis() {
	FILE *fp;
	char buf[MAXLEN];
	fp= fopen("/proc/net/dev","r");	
	if(fp==NULL) {
		fprintf(stderr,"Could not open /proc/net/dev\n");
		sleep(30);
		return;
	}
	while(fgets(buf,MAXLEN,fp)!=NULL) {
		unsigned long int rp,rb,tp,tb,ign;
		char *p,*devname;
		p=(char*)strchr(buf,':');
                if(p==(char*)0) continue;
                *p=0; p++;
		if(p==(char*)0) continue;
		devname=buf;
		while(*devname==' ') devname++;
		if(strcmp(devname,"dummy0")==0 ||
                           strcmp(devname,"tunl0")==0 ||
                           strcmp(devname,"gre0")==0 ||
                           strcmp(devname,"lo")==0)  continue;

		if(sscanf(p,"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
			&rb,&rp,&ign,&ign,&ign,&ign,&ign,&ign,&tb,&tp)!=10) {
			fprintf(stderr,"Could not open /proc/net/dev\n");
			sleep(30);
			continue;
		}
		AddCount(devname,rp,rb,tp,tb);
	} 
	fclose(fp);
}


int main()
{
	setsid();
	if(pv_init()<0) exit(-1);
	if(getshm(1)<0) exit(-1);
	//if(fork()) exit(0);
	while(1) {
		dostatis();
		sleep(1);
	}
	return 0;
}
