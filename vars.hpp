struct cs_window
{
	int windowwidth, windowheight;
};

namespace vars
{
    inline std::uintptr_t module_client = 0;
    inline std::uintptr_t module_engine2 = 0;
    inline std::uintptr_t cs2_entitylist = 0;
    inline int cs2_buildnumber = 0;
    inline cs_window cs2_window = { 0, 0 };  
}