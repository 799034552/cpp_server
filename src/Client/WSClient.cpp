#include"WSClient.h"
#include"WSPool.h"
#include<cstring>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include<iostream>
using std::cout;
using std::endl;
const string WSClient::GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

int base64_encode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    BUF_MEM *bptr = NULL;
    size_t size = 0;

    if (in_str == NULL || out_str == NULL)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in_str, in_len);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out_str, bptr->data, bptr->length);
    out_str[bptr->length-1] = '\0';
    size = bptr->length;

    BIO_free_all(bio);
    return size;
}

string get_send_head(frame_head* head)
{
    char response_head[12];
    int head_length = 0;
    if(head->payload_length<126)
    {
        //response_head = (char*)malloc(2);
        response_head[0] = 0x81;
        response_head[1] = head->payload_length;
        head_length = 2;
    }
    else if (head->payload_length<0xFFFF)
    {
        //response_head = (char*)malloc(4);
        response_head[0] = 0x81;
        response_head[1] = 126;
        response_head[2] = (head->payload_length >> 8 & 0xFF);
        response_head[3] = (head->payload_length & 0xFF);
        head_length = 4;
    }
    else
    {
      return "";
    }
    response_head[head_length] = '\0';
    string res = response_head;
    return res;
}
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";
std::string m_base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
 
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
 
			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
 
	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';
 
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
 
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
 
		while ((i++ < 3))
			ret += '=';
 
	}
 
	return ret;
}

void umask(char *data,int len,char *mask)
{
    int i;
    for (i=0;i<len;++i)
        *(data+i) ^= *(mask+(i%4));
}



WSClient::WSClient(int fd_,std::unordered_map<string, string> &http_data):
  Client(fd_),
  read_close(false),
  read_state(READ_STATE::READ_OPEN),
  parse_state(PARSE_STATE::TWO_CHAR),
  is_send_over(true)
{
  req_head.swap(http_data);
  read_fn = std::bind(&WSClient::ws_read, this);
  write_fn = std::bind(&WSClient::ws_write, this);
  set_event(EPOLLIN|EPOLLET);
  live_time = 60*10;
  f_head.fin = 1;
  f_head.mask = 1;
  f_head.opcode = 0x1;
}
void WSClient::ws_read()
{
  cout<<"ws read------------------"<<endl;
  int n = read_all(read_close);
  if (n < 0) {
    is_close = true;
    return;
  }
  else if (n == 0 && !read_close)
    return;
  char one_char;
  while(rec_buf.size()!= 0)
  {
    switch (parse_state)
    {
    case PARSE_STATE::TWO_CHAR:
      if(rec_buf.size()<2) return;
      one_char = rec_buf[0];
      f_head.fin = (one_char & 0x80) == 0x80;
      f_head.opcode = one_char & 0x0F;
      one_char = rec_buf[1];
      f_head.mask = (one_char & 0x80) == 0X80;
      f_head.payload_length = one_char & 0x7F;
      parse_state = PARSE_STATE::PAY_LOAD_Length;
      rec_buf = rec_buf.substr(2);
      break;
    case PARSE_STATE::PAY_LOAD_Length:
      if (f_head.payload_length == 126)
      {
        if (rec_buf.size() < 2) return;
        f_head.payload_length = (rec_buf[0]&0xFF) << 8 | (rec_buf[1]&0xFF);
        rec_buf = rec_buf.substr(2);
      }
      else if (f_head.payload_length == 127)
      {
        if (rec_buf.size() < 8) return;
          char temp;

          for(int i=0;i<4;i++)
          {
              temp = rec_buf[i];
              rec_buf[i] = rec_buf[7-i];
              rec_buf[7-i] = temp;
          }
          memcpy(&(f_head.payload_length),&rec_buf,8);
          rec_buf = rec_buf.substr(8);
      }
      parse_state = PARSE_STATE::Masking_Key;
      break;
    case PARSE_STATE::Masking_Key:
      if (rec_buf.size() < 4) return;
      strncpy(f_head.masking_key, &rec_buf[0], 4);
      parse_state = PARSE_STATE::PAY_LOAD;
      break;
    case PARSE_STATE::PAY_LOAD:
      if (rec_buf.size() < f_head.payload_length) return;
      umask(&rec_buf[0], rec_buf.size(),f_head.masking_key);
      read_state = READ_STATE::READ_OK;
      break;
    }
    if (read_state == READ_STATE::READ_OK)
      break;
  }
  if (read_state == READ_STATE::READ_OK)
  {
    update_event(EPOLLOUT|EPOLLET);
    cout<<"i get ws data------------------"<<endl;
    cout<<rec_buf<<endl;
    if (get_fn)
      get_fn(rec_buf);
    //emit(rec_buf);
    reset();
    //rec_buf.clear();
  }
  else if (read_state == READ_STATE::READ_BAD)
    is_close = true;
}

void WSClient::ws_write()
{
  if (send_buf.size() > 0)
  {
    cout<<"ws write-----------------------"<<endl;
    cout<<send_buf<<endl;
    if (is_send_over) {
      f_head.payload_length = send_buf.size();
      send_buf =  get_send_head(&f_head) + send_buf;
      is_send_over = false;
    }

    int n = write(fd,&send_buf[0], send_buf.size());
    if (n > 0)
    {
      if (n == send_buf.size())
      {
        is_send_over = true;
        update_event(EPOLLIN|EPOLLET);
        send_buf.clear();
        if (read_close)
          is_close = true;
      } else {
        send_buf = send_buf.substr(n);
      }
    }
    else if (n < 0)
      is_close = true;
    
  } else
    update_event(EPOLLIN|EPOLLET);
}

void WSClient::emit(string s)
{
  cout<<"emit start--------------------"<<endl;
  send_buf = s;
  
  update_event(EPOLLOUT|EPOLLET);
  cout<<"emit end--------------------"<<endl;
  
}

void WSClient::reset()
{
  read_state = READ_STATE::READ_OPEN;
  parse_state = PARSE_STATE::TWO_CHAR;
  rec_buf.clear();
}

WSClient::READ_STATE WSClient::read_head()
{

}