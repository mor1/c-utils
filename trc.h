/*********************************************************************
 * Trace macros
 *
 * Copyright (C) 2000 Richard Mortier <mort@cantab.net>.  All Rights
 * Reserved.
 *
 * Based on Tim Deegan's (tjd21) and Keir Fraser's (kaf24).
 *
 * Currently broken (hey, stuff bitrots after 8 years of not being
 * touched). 
 **/

#ifdef __KERNEL__
# define dprintf printk
#else
# define dprintf printf
#endif

#define ERROR(fmt,args...) dprintf("*** [%s:%i]:%s: " ## fmt,   \
                                   __FILE__, __LINE__,          \
                                   __PRETTY_FUNCTION__ , ## args)

#ifdef _ENTER_EXIT_
# define ENTER   dprintf("[%s:%i]: +++ %s\n",				\
                         __FILE__, __LINE__, __PRETTY_FUNCTION__)
# define LEAVE   dprintf("[%s:%i]: --- %s\n",				\
                         __FILE__, __LINE__, __PRETTY_FUNCTION__)
# define RETURN  LEAVE; return
#else
# define ENTER
# define LEAVE
# define RETURN  return
#endif // _ENTER_EXIT_

#ifdef _TRACE_
# define TRC(fmt,args...) dprintf("### [%s:%i]:%s: " ## fmt,		\
                                  __FILE__, __LINE__,               \
                                  __PRETTY_FUNCTION__ , ## args)
#else
# define TRC(fmt,args...)
#endif // _TRACE_
