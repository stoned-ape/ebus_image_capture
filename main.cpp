#define WIN32
#define _CONSOLE
#define _WIN32_WINNT 0x0501
#define _UNICODE
#define UNICODE
#define _AFXDL
#define _HAS_STD_BYTE 0
#define WIN32_LEAN_AND_MEAN 

typedef unsigned char byte;
#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvBuffer.h>
#include <assert.h>
#include <stdlib.h>

PV_INIT_SIGNAL_HANDLER();

struct rgba8{
    uint8_t b,g,r,a;
};

#pragma pack(2)
struct bmp_header{
    uint16_t magic;
    uint32_t file_size;
    uint32_t app;
    uint32_t offset;
    uint32_t info_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pix;
    uint32_t comp;
    uint32_t comp_size;
    uint32_t xres;
    uint32_t yres;
    uint32_t cols_used;
    uint32_t imp_cols;
};

void make_bmp(rgba8 *buf,int w,int h,const char *fname){
    puts("make_bmp()");
    bmp_header head;
    assert(offsetof(bmp_header,file_size)==2);
    assert(sizeof(head)==54);
    memset(&head,0,sizeof(head));

    head.magic=0x4d42;
    head.offset=sizeof(bmp_header);
    head.info_size=40;
    head.width=w;
    head.height=h;
    head.planes=1;
    head.bits_per_pix=32;
    head.comp_size=head.width*head.height*head.bits_per_pix/8;
    head.file_size=sizeof(bmp_header)+head.comp_size;

    FILE *bmp=fopen(fname,"wb");
    assert(fwrite(&head,1,sizeof(head),bmp)==sizeof(head));
    assert(fwrite(buf,1,head.comp_size,bmp)==head.comp_size);
    fclose(bmp);
}

rgba8 *transform_buffer(uint8_t *ptr,int w,int h){
    rgba8 *buf=new rgba8[w*h/4];
    for(int i=0;i<w/2;i++){
        for(int j=0;j<h/2;j++){
            buf[i+j*w/2].b=ptr[(2*i+1)+(2*j+1)*w];
            buf[i+j*w/2].g=ptr[(2*i+1)+(2*j+0)*w];
            buf[i+j*w/2].r=ptr[(2*i+0)+(2*j+0)*w];
        }
    }
    return buf;
}

#define PRINT(val,type) printf("%s:\t"type"\n",#val,val);

int main(){
    PRINT(sizeof(PvBuffer),"%lld");
    PRINT(sizeof(PvImage),"%lld");
    PvDevice *device=NULL;
    PvStream *stream=NULL;
    PV_SAMPLE_INIT();
    PvString conn_id="267601CAAF2C";
    PvSelectDevice(&conn_id);
    printf("id: %s\n",(const char*)conn_id);
    PvResult result;
    cout<<"connecting to device.\n";
    device=PvDevice::CreateAndConnect(conn_id,&result);
    assert(device);
    assert(result.IsOK());
    cout<<"opening stream from device.\n";
    stream=PvStream::CreateAndOpen(conn_id,&result);
    assert(stream);
    assert(result.IsOK());
    PvBuffer pvbuf1;
    pvbuf1.Alloc(static_cast<uint32_t>(device->GetPayloadSize()));
    stream->QueueBuffer(&pvbuf1);
    PvGenParameterArray *params=device->GetParameters();
    PvGenCommand *start=dynamic_cast<PvGenCommand*>(params->Get("AcquisitionStart"));
    device->StreamEnable();
    start->Execute();
    PvBuffer *pvbuf2=NULL;
    PvResult op_result;
    result=stream->RetrieveBuffer(&pvbuf2,&op_result,1000);
    assert(pvbuf2==&pvbuf1);
    assert(result.IsOK());
    assert(op_result.IsOK());
    assert(pvbuf2);
    uint8_t *ptr=pvbuf1.GetDataPointer();
    assert(ptr);
    PvImage *image = pvbuf1.GetImage();
    int w=image->GetWidth();
    int h=image->GetHeight();
    rgba8 *buf=transform_buffer(ptr,w,h);
    make_bmp(buf,w/2,h/2,"img.bmp");
    system("img.bmp");
    delete []buf;
}
