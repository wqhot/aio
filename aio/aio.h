// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 {0}_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// {0}_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef AIO_EXPORTS
#define AIO_API __declspec(dllexport)
#else
#define AIO_API __declspec(dllimport)
#endif

// 此类是从 dll 导出的
class AIO_API Caio {
public:
	Caio(void);
	// TODO: 在此处添加方法。
};

extern AIO_API int naio;

AIO_API int fnaio(void);

extern "C"
{
	__declspec(dllexport) long aio_init(char* dev_name, float delta, short range, long simple_times);
	__declspec(dllexport) void get_available_buffer_size(int* length);
	__declspec(dllexport) void get_buffer(int num, float* data);
	__declspec(dllexport) int exit_aio();
	__declspec(dllexport) long aio_start();
}
