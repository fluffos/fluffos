#ifndef _OUTBUF_H_
#define _OUTBUF_H_

struct outbuffer_t {
  size_t real_size;
  char *buffer;
};

void outbuf_zero(outbuffer_t *);
void outbuf_add_internal(outbuffer_t *, const std::string format, fmt::format_args);
template <typename... Args> void outbuf_add(outbuffer_t *outbuf, const std::string format, Args&& ...args)
{
    outbuf_add_internal(outbuf, format, fmt::make_format_args(std::forward<Args>(args)...));
}
void outbuf_addchar(outbuffer_t *, char);
void outbuf_fix(outbuffer_t *);
size_t outbuf_extend(outbuffer_t *, size_t);

#endif
