#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>

// for an explanation of these controversial definitions, refer to masteryeti.com
#define ref &
#define addr *
#define nulladdr nullptr

#define BUFFER_FILL_CHAR '\0' // buffer is initialized/filled with this char, in case the provided buffer is larger than the by ioctl written value
#define C_COMPILER "/bin/gcc" // any installed C compiler should do
#define SEARCH_INCLUDE_PATHS "/usr/include/linux /usr/include/sound /usr/include" // first linux to speed up search results

// taken from https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec(const char addr cmd_addr)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd_addr, "r"), pclose);
    if(!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nulladdr)
    {
        result += buffer.data();
    }
    return result;
}

int test_string_is_number(const char addr str)
{
	size_t i = 0;
	char c;
	while((c = str[i++]) != '\0')
	{
		if(c < '0' || c > '9')
		{
			return 0;
		}
	}
	return i > 0;
}

size_t parse_arg(char addr str_addr, void addr addr data_addr_addr, size_t index, int addr is_struct_addr)
{
    size_t len = std::strlen(str_addr);
    
    if(len == 0)
    {
        return 0;
    }
    
    char first_chr = *(str_addr + 0);
    char last_chr = *(str_addr + len - 1);
    
    void addr value_addr = nulladdr;
    size_t value_len = 0;
    
    // if first and last character is the same, and not a number
    if(first_chr == last_chr && (first_chr < '0' || first_chr > '9'))
    {
        // then parse as string
        
        // calculate new length (trim first and last characters -2) (add +1 for \0)
        value_len = sizeof(*str_addr) * (len + 1 - 2); // +1 for \0, -2 for first and last character
        
        // allocate memory, -2 for trimming first and last characters, and +1 for \0
        value_addr = std::malloc(value_len);
        
        // copy the relevant characters from str_addr
        std::memcpy(value_addr, str_addr + 1, len - 2);
        
        // set \0 at the end
        *((char addr) value_addr + len - 1) = '\0';
    }
    else
    {
        // check if number (int, float, etc.)
        int is_number = 1;
        int has_literal = -1;
        int has_dot = -1;
        
        for(size_t i=0;i<len;++i)
        {
            char c = *(str_addr + i);
            
            if(c == '.') // c is a dot
            {
                if(has_dot >= 0 || has_literal >= 0)
                {
                    // multiple dots or came after literal, parse error
                    is_number = 0;
                    break;
                }
                
                has_dot = i;
            }
            else if(c >= '0' && c <= '9') // c is non-numeric
            {
                if(has_literal >= 0)
                {
                    // a number after the literal was given is wrong, parse error
                    is_number = 0;
                    break;
                }
            }
            else // c is numeric
            {
                has_literal = i;
            }
        }
        
        if(is_number)
        {
            size_t literal_len = 0;
            char addr literal_addr = nulladdr;
            
            if(has_literal >= 0)
            {
                literal_len = sizeof(char) * (len - has_literal + 1); // +1 for \0
                
                literal_addr = (char addr) std::malloc(literal_len);
                
                std::memcpy(literal_addr, str_addr + has_literal, literal_len);
                
                // force all literals to lowercase for simpler parsing
                for(size_t i=0;i<literal_len;++i)
                {
                    *(literal_addr + i) = std::tolower(*(literal_addr + i));
                }
            }
            else if(has_dot >= 0)
            {
                literal_len = sizeof(char) * (1 + 1); // +1 for \0
                
                literal_addr = (char addr) std::malloc(literal_len);
                
                std::memcpy(literal_addr, &"f", literal_len);
            }
            else
            {
                literal_len = sizeof(char) * (1 + 1); // +1 for \0
                
                literal_addr = (char addr) std::malloc(literal_len);
                
                std::memcpy(literal_addr, &"i", literal_len);
            }
            
            if(*(literal_addr + 1) == '\0')
            {
                if(*literal_addr == 'b')
                {
                    value_len = sizeof(char) * std::strtoll((const char addr) str_addr, nullptr, 10);
                    
                    value_addr = std::malloc(value_len);
                    
                    // mark this is a buffer/struct to be written to stdout later
                    *is_struct_addr = 1;
                    
                    // let's initialize these buffers with question marks:
                    for(size_t i=0;i<value_len;++i)
                    {
                        *((char addr) value_addr + i) = BUFFER_FILL_CHAR;
                    }
                }
                else if(*literal_addr == 'i')
                {
                    value_len = sizeof(int);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((int addr) value_addr) = std::atoi((const char addr) str_addr);
                }
                else if(*literal_addr == 'f')
                {
                    value_len = sizeof(float);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((float addr) value_addr) = std::atof((const char addr) str_addr);
                }
                else if(*literal_addr == 'd')
                {
                    value_len = sizeof(float);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((float addr) value_addr) = std::atof((const char addr) str_addr);
                }
                else if(*literal_addr == 'z')
                {
                    value_len = sizeof(ssize_t);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((size_t addr) value_addr) = std::atoi((const char addr) str_addr);
                }
                else if(*literal_addr == 'l')
                {
                    value_len = sizeof(long int);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((long int addr) value_addr) = std::atol((const char addr) str_addr);
                }
            }
            else if(*literal_addr == 'l' && *(literal_addr + 1) == 'l' && *(literal_addr + 2) == '\0')
            {
                value_len = sizeof(long long int);
                
                value_addr = std::malloc(value_len);
                
                *((long long int addr) value_addr) = std::atoll((const char addr) str_addr);
            }
            else if(*literal_addr == 'u')
            {
                if(*(literal_addr + 1) == '\0' || *(literal_addr + 1) == 'i')
                {
                    value_len = sizeof(unsigned int);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((unsigned int addr) value_addr) = std::atoi((const char addr) str_addr);
                }
                else if(*(literal_addr + 1) == 'l')
                {
                    if(*(literal_addr + 2) == '\0')
                    {
                        value_len = sizeof(unsigned long int);
                        
                        value_addr = std::malloc(value_len);
                        
                        *((unsigned long int addr) value_addr) = std::atol((const char addr) str_addr);
                    }
                    else if(*(literal_addr + 2) == 'l' && *(literal_addr + 3) == '\0')
                    {
                        value_len = sizeof(unsigned long long int);
                        
                        value_addr = std::malloc(value_len);
                        
                        *((unsigned long long int addr) value_addr) = std::atoll((const char addr) str_addr);
                    }
                }
                else if(*(literal_addr + 1) == 'z')
                {
                    value_len = sizeof(size_t);
                    
                    value_addr = std::malloc(value_len);
                    
                    *((size_t addr) value_addr) = std::atoll((const char addr) str_addr);
                }
            }
        }
    }
    
    if(value_addr != nulladdr)
    {
        // create new array of suitable length
        void addr new_array = std::malloc(index + value_len);
        // copy old array into new one
        std::memcpy(new_array, *data_addr_addr, index);
        // free previous array
        if(*data_addr_addr != nullptr) free(*data_addr_addr);
        // get the pointer to the new array
        *data_addr_addr = new_array;
        
        // copy value into new array at the right index
        std::memcpy((char addr) *data_addr_addr + index, value_addr, value_len);
        
        // free value_addr after copy
        free(value_addr);
    }
    
    return value_len;
}

void print_help()
{
    std::cerr << "Usage: ioctl [file|device] [request] [string|number|struct...]" << std::endl
              << std::endl
              << "The argument type is automatically parsed like so:" << std::endl
              << " [string]: Xsome stringX (must start and end with any same non-numeric symbol)" << std::endl
              << " [struct]: 64B (indicating the size in bytes of the struct)" << std::endl
              << " [number]: 10ull (use any regular integer literal, defaults to int or float)" << std::endl
              << std::endl
              << "Note: On-the-fly compilation is used to resolve the request parameter if it is" << std::endl
              << "not formatted as an unsigned long. This will:" << std::endl
              << " - execute a recursive grep on /usr/include to find the request definition in the header file;" << std::endl
              << " - create and compile a /tmp/XXXXXXXX.c file (using gcc);" << std::endl
              << " - run the compiled executable in order to find the unsigned long int value;" << std::endl
              << " - generate a warning (in stderr) with the unsigned long int value which may be" << std::endl
              << "   used for - more efficient - subsequent calls." << std::endl
              << std::endl
              << "In order to find out the actual unsigned long int value without" << std::endl
              << "executing ioctl, use - as only request argument. In this case," << std::endl
              << "the unsigned long int value will be directly written to stdout." << std::endl
              << std::endl
              << "Example usage:" << std::endl
              << std::endl
              << "Note the struct definition <linux/videodev2.h>:" << std::endl
              << "  struct v4l2_capability {" << std::endl
              << "      __u8    driver[16];" << std::endl
              << "      __u8    card[32];" << std::endl
              << "      __u8    bus_info[32];" << std::endl
              << "      __u32   version;" << std::endl
              << "      __u32   capabilities;" << std::endl
              << "      __u32   device_caps;" << std::endl
              << "      __u32   reserved[3];" << std::endl
              << "  };" << std::endl
              << std::endl
              << "For efficiency, resolve the request parameter to an unsigned long:" << std::endl
              << std::endl
              << "  > ioctl /dev/video0 VIDIOC_QUERYCAP -" << std::endl
              << "  2154321408" << std::endl
              << std::endl
              << "Use dd to extract the v4l2_capability.card string:" << std::endl
              << std::endl
              << "  > ioctl /dev/video0 2154321408 104B | dd bs=1 skip=16 count=32 2>&-; echo" << std::endl
              << "  USB2.0 HD UVC WebCam: USB2.0 HD" << std::endl
              << std::endl
              << "Alternatively inefficiently but directly with an unknown buffer size:" << std::endl
              << "  > ioctl /dev/video0 VIDIOC_QUERYCAP 10000B | dd bs=8 skip=2 count=4 2>&-; echo" << std::endl
              << "  warning: Used on-the-fly compilation to resolve request parameter" << std::endl
              << "  (VIDIOC_QUERYCAP). To optimize, run:" << std::endl
              << "  ./ioctl /dev/video0 2154321408 10000B" << std::endl
              << "  USB2.0 HD UVC WebCam: USB2.0 HD" << std::endl
              << std::endl;
}

// for e.g.: VIDIOC_QUERYCAP
// run: ioctl /dev/video0 2154321408 104B

int main(int argc, char addr argv[])
{
    // check -h,--help parameter:
    for(size_t i=1;i<argc;++i)
    {
        char addr arg_addr = argv[i];
        if(strcmp(arg_addr, "-h") == 0 || strcmp(arg_addr, "--help") == 0)
        {
            print_help();
            return 0;
        }
    }
    
    
    
    // read file path, and open filedescriptor
    const char addr file = argv[1];
    
    // O_NONBLOCK: ioctl(2): "In order to use this call, one needs an open file descriptor. Often the open(2) call has unwanted side effects, that can be avoided under Linux by giving it the O_NONBLOCK flag."
	int fd = open(file, O_RDWR | O_NONBLOCK);
    if(fd < 0 && (errno == EPERM || errno == EACCES))
    {
        fd = open(file, O_RDONLY | O_NONBLOCK);
    }
    if(fd < 0)
    {
        std::cerr << "error: Failed to open file (" << file << ")." << std::endl;
        return 1;
    }
    
    
    
    // parse (and resolve) request parameter
    char addr request_arg = argv[2];
    unsigned long request = 0;
    {
        if(test_string_is_number(request_arg))
        {
            request = std::strtoul(request_arg, nullptr, 10);
        }
        else
        {
            std::string cmd_arg = request_arg;
            std::string cmd = 
            "tmp=\"$(mktemp /tmp/XXXXXXXX)\"\n"
            "echo -e \""
                "#include <stdio.h>\n"
                "#include <$(grep --line-buffered -r -l -E '^\\s*#define\\s+" + cmd_arg + "\\s+' " SEARCH_INCLUDE_PATHS " | head -n 1)>\n"
                "int main(){printf(\"\\\"%u\\\"\", " + cmd_arg + ");}"
                "\" >\"$tmp.c\"\n"
            C_COMPILER " \"$tmp.c\" -o \"$tmp\"\n"
            "\"$tmp\"\n"
            "rm -f \"$tmp.c\" \"$tmp\"";
            std::string cmd_result = exec(cmd.c_str());
            if(!test_string_is_number(cmd_result.c_str()))
            {
                std::cerr << "error: Failed to resolve request parameter (" << cmd_arg << ", " << cmd_result.c_str() << ")." << std::endl;
                return 1;
            }
            request = std::stol(cmd_result);
            
            if(argc == 4 && std::strcmp(argv[3], "-") == 0)
            {
                std::cout << request << std::endl;
                return 1;
            }
            
            std::cerr << "warning: Used on-the-fly compilation to resolve request parameter (" << cmd_arg << "). To optimize, run:" << std::endl
                    << argv[0] << " " << argv[1] << " " << request;
            for(size_t i=3;i<argc;++i)
            {
                std::cerr << " " << argv[i];
            }
            std::cerr << std::endl;
        }
    }
    
    
    
    // build void* for ioctl arguments
	std::vector<size_t> outputbuffers;
	
	void addr ioctl_args_addr = nullptr;
	size_t ioctl_args_size = 0;
	for(size_t i=3;i<argc;++i)
	{
		char addr arg_addr = argv[i];
        int is_struct = 0;
        size_t arg_size = parse_arg(arg_addr, &ioctl_args_addr, ioctl_args_size, &is_struct);
        
        if(is_struct)
        {
            outputbuffers.push_back(ioctl_args_size);
            outputbuffers.push_back(arg_size);
        }
        
        ioctl_args_size += arg_size;
	}
	
	
    
    // actually execute ioctl
	int result = ioctl(fd, request, ioctl_args_addr);
	
    
    
    // buffers with B literal will be printed to stdout and concatenated
    for(int i=0;i<outputbuffers.size();++i)
    {
        size_t outbuf_index = outputbuffers[i];
        size_t outbuf_size = outputbuffers[++i];
        
        const void addr outbuf_addr = (const char addr) ioctl_args_addr + outbuf_index;
        
        write(STDOUT_FILENO, outbuf_addr, outbuf_size);
    }
	
	
	// exit code is the result of ioctl
	return result;
}
