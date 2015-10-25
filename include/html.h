#define ITEM_CHECKED			" checked=\"checked\""
#define ITEM_SELECTED			" selected=\"selected\""

#define HTML_DIR				"&lt;dir&gt;"
#define HTML_BUTTON_FMT			"%s%s\" %s'%s';\">"
#define HTML_BUTTON				" <input type=\"button\" value=\""
#define HTML_ONCLICK			"onclick=\"location.href="
#define HTML_INPUT(n, v, m, s)	"<input name=\"" n "\" type=\"text\" value=\"" v "\" maxlength=\"" m "\" size=\"" s "\">"

static char h2a(char hex);
static void urlenc(char *dst, char *src, u8 rel_mode);
static void htmlenc(char *dst, char *src, u8 cpy2src);
static void utf8enc(char *dst, char *src, u8 cpy2src);

static void add_radio_button(const char *name, const char *value, const char *id, const char *label, const char *sufix, bool checked, char *buffer);
static void add_check_box(const char *name, const char *value, const char *label, const char *sufix, bool checked, char *buffer);
static void add_option_item(const char *value, const char *label, bool selected, char *buffer);

static void prepare_header(char *header, char *param, u8 is_binary);

static void get_value(char *text, char *url, u16 size);
static int val(const char *c);


static char h2a(char hex)
{
	char c = hex;
	if(c>=0 && c<=9)
		c += '0';
	else if(c>=10 && c<=15)
		c += 0x57; //a-f
	return c;
}

static void urlenc(char *dst, char *src, u8 rel_mode)
{
	size_t j=0;
    size_t n=strlen(src);
	for(size_t i=0; i<n; i++,j++)
	{
		     if(src[i]==' ') {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '0';}
		else if(src[i]==':' && rel_mode) {dst[j++] = '%'; dst[j++] = '3'; dst[j] = 'A';}
		else if(src[i] & 0x80)
		{
			dst[j++] = '%';
			dst[j++] = h2a((unsigned char)src[i]>>4);
			dst[j] = h2a(src[i] & 0xf);
		}
		else if(src[i]==34) {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '2';}
		else if(gmobile_mode && src[i]==39) {dst[j++] = '%'; dst[j++] = '2'; dst[j] = '7';}
		else dst[j] = src[i];
	}
	dst[j] = '\0';
}

static void htmlenc(char *dst, char *src, u8 cpy2src)
{
	size_t j=0;
    size_t n=strlen(src); char tmp[8]; memset(dst, 4*n, 0); u8 t, c;
	for(size_t i=0; i<n; i++)
	{
		if(src[i] & 0x80)
		{
			dst[j++] = '&';
			dst[j++] = '#';
			sprintf(tmp, "%i;", (int)(unsigned char)src[i]); t=strlen(tmp); c=0;
			while(t--) {dst[j++] = tmp[c++];}
		}
		else dst[j++] = src[i];
	}
	dst[j] = '\0';

	if(cpy2src) strncpy(src, dst, MAX_LINE_LEN);
}

static void utf8enc(char *dst, char *src, u8 cpy2src)
{
	size_t j=0, n=strlen(src); u16 c;
	for(size_t i=0; i<n; i++)
	{
		c=(src[i]&0xFF);

		if(!(c & 0xff80)) dst[j++]=c;
		else //if(!(c & 0xf800))
		{
			dst[j++]=0xC0|(c>>6);
			dst[j++]=0x80|(0x3F&c);
		}
/*
		else
		{
			dst[j++]=0xE0|(0x0F&(c>>12));
			dst[j++]=0x80|(0x3F&(c>>06));
			dst[j++]=0x80|(0x3F&(c    ));
		}
*/
	}
	dst[j] = '\0';

	if(cpy2src) strncpy(src, dst, MAX_LINE_LEN);
}
/*
static void utf8dec(char *dst, char *src, u8 cpy2src))
{
	size_t j=0;
	size_t n=strlen(src); u8 c;
	for(size_t i=0; i<n; i++)
	{
		c=(src[i]&0xFF);
		if(c<0x80)
			dst[j++]=c;
		else if(c & 0x20)
			dst[j++]=(((src[i++] & 0x1F)<<6)+(c & 0x3F));
		else
			dst[j++]=(((src[i++] & 0xF)<<12)+((src[i++] & 0x3F)<<6)+(c & 0x3F));
	}
	dst[j] = '\0';

	if(cpy2src) strncpy(src, dst, MAX_LINE_LEN);
}
*/
static void add_radio_button(const char *name, const char *value, const char *id, const char *label, const char *sufix, bool checked, char *buffer)
{
	char templn[MAX_LINE_LEN];
	sprintf(templn, "<label><input type=\"radio\" name=\"%s\" value=\"%s\" id=\"%s\"%s/> %s%s</label>", name, value, id, checked?ITEM_CHECKED:"", label, (!sufix)?"<br>":sufix);
	strcat(buffer, templn);
}

static void add_check_box(const char *name, const char *value, const char *label, const char *sufix, bool checked, char *buffer)
{
	char templn[MAX_LINE_LEN];
	char clabel[MAX_LINE_LEN];
	char *p;
	strcpy(clabel, label);
	p=strstr(clabel, AUTOBOOT_PATH);
	if(p!=NULL)
	{
		p[0]=0;
		sprintf(templn, HTML_INPUT("autop", "%s", "255", "40"), webman_config->autoboot_path);
		strcat(clabel, templn);
		p=strstr(label, AUTOBOOT_PATH)+strlen(AUTOBOOT_PATH);
		strcat(clabel, p);
	}
	sprintf(templn, "<label><input type=\"checkbox\" name=\"%s\" value=\"%s\"%s/> %s</label>%s", name, value, checked?ITEM_CHECKED:"", clabel, (!sufix)?"<br>":sufix);
	strcat(buffer, templn);
}

static void add_option_item(const char *value, const char *label, bool selected, char *buffer)
{
	char templn[MAX_LINE_LEN];
	sprintf(templn, "<option value=\"%s\"%s/>%s</option>", value, selected?ITEM_SELECTED:"", label);
	strcat(buffer, templn);
}

static void prepare_header(char *header, char *param, u8 is_binary)
{
	bool set_base_path = false;

	strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: \0");
	if(is_binary==1)
	{
		if(!extcasecmp(param, ".htm", 4) || !extcasecmp(param, ".html", 5) || strcasestr(param, ".shtm"))
			{strcat(header, "text/html"); set_base_path = true;}
		else
		if(!extcasecmp(param, ".jpg", 4) || !extcasecmp(param, ".jpeg", 5) || !extcmp(param, ".STH", 4))
			strcat(header, "image/jpeg");
		else
		if(!extcasecmp(param, ".png", 4))
			strcat(header, "image/png");
		else
		if(!extcasecmp(param, ".txt", 4) || !extcasecmp(param, ".log", 4) || !extcasecmp(param, ".ini", 4) || !extcmp(param, ".HIP", 4) || !extcmp(param, ".HIS", 4))
			strcat(header, "text/plain");
		else
		if(!extcasecmp(param, ".css", 4))
			strcat(header, "text/css");
		else
		if(!extcasecmp(param, ".js", 3))
			strcat(header, "text/javascript");
		else
		if(!extcasecmp(param, ".svg", 4))
			strcat(header, "image/svg+xml");
#ifndef LITE_EDITION
		else
		if(!extcasecmp(param, ".gif", 4))
			strcat(header, "image/gif");
		else
		if(!extcasecmp(param, ".avi", 4))
			strcat(header, "video/x-msvideo");
		else
		if(!extcasecmp(param, ".mkv", 4))
			strcat(header, "video/x-matroska");
		else
		if(!extcasecmp(param, ".mp4", 4))
			strcat(header, "video/mp4");
		else
		if(!extcasecmp(param, ".mpg", 4) || !extcasecmp(param, ".mp2", 4) || strcasestr(param, ".mpe"))
			strcat(header, "video/mpeg");
		else
		if(!extcasecmp(param, ".vob", 4))
			strcat(header, "video/vob");
		else
		if(!extcasecmp(param, ".wmv", 4))
			strcat(header, "video/x-ms-wmv");
		else
		if(!extcasecmp(param, ".mov", 4))
			strcat(header, "video/quicktime");
		else
		if(!extcasecmp(param, ".mp3", 4))
			strcat(header, "audio/mpeg");
		else
		if(!extcasecmp(param, ".wav", 4))
			strcat(header, "audio/x-wav");
		else
		if(!extcasecmp(param, ".bmp", 4))
			strcat(header, "image/bmp");
		else
		if(!extcasecmp(param, ".tif", 4))
			strcat(header, "image/tiff");
		else
		if(!extcasecmp(param, ".zip", 4))
			strcat(header, "application/zip");
		else
		if(!extcasecmp(param, ".pdf", 4))
			strcat(header, "application/pdf");
		else
		if(!extcasecmp(param, ".swf", 4))
			strcat(header, "application/x-shockwave-flash");
#endif
		else
			strcat(header, "application/octet-stream");
	}
	else
		{strcat(header, "text/html"); set_base_path = true;}

	if(set_base_path && param[0]=='/') {strncpy(html_base_path, param, MAX_PATH_LEN); html_base_path[MAX_PATH_LEN]=0; html_base_path[strrchr(html_base_path, '/')-html_base_path]=0;}

	strcat(header, "\r\n");
}

static void get_value(char *text, char *url, u16 size)
{
	u16 n;
	for(n=0;n<size;n++)
	{
		if(url[n]=='&' || url[n]==0) break;
		if(url[n]=='+') url[n]=' ';
		text[n]=url[n];
	}
	text[n]=0;
}

static int val(const char *c)
{
    int previous_result=0, result=0;
    int multiplier=1;

    if(c && *c == '-')
    {
        multiplier = -1;
        c++;
    }

    while(*c)
    {
        if(*c < '0' || *c > '9') return result * multiplier;

        result *= 10;
        if(result < previous_result)
            return(0);
        else
            previous_result *= 10;

        result += *c - '0';
        if(result < previous_result)
            return(0);
        else
            previous_result += *c - '0';

        c++;
    }
    return(result * multiplier);
}

