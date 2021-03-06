#define llodepng
#define LUA_LIB

#include <lua/lauxlib.h>
#include <lua/lualib.h>
#include <lua/lua.h>

#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"

#define PNG_METATABLE "lodepng.png"

static int png_at(lua_State *L)
{
    lua_getfield(L, 1, "width");
    lua_Integer width = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "height");
    lua_Integer height = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "data");
    unsigned char *image = (unsigned char *)lua_touserdata(L, -1);

    lua_Integer x = luaL_checkinteger(L, 2);
    lua_Integer y = luaL_checkinteger(L, 3);

    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        lua_pushinteger(L, 0), lua_pushinteger(L, 0), lua_pushinteger(L, 0),
            lua_pushinteger(L, 0);
        return 4;
    }

    lua_Integer index = (y * width + x) * 4;
    lua_pushinteger(L, (unsigned int)image[index]);
    lua_pushinteger(L, (unsigned int)image[index + 1]);
    lua_pushinteger(L, (unsigned int)image[index + 2]);
    lua_pushinteger(L, (unsigned int)image[index + 3]);

    return 4;
}

static int png_set(lua_State *L)
{
    lua_getfield(L, 1, "width");
    lua_Integer width = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "height");
    lua_Integer height = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "data");
    unsigned char *image = (unsigned char *)lua_touserdata(L, -1);

    lua_Integer x = luaL_checkinteger(L, 2);
    lua_Integer y = luaL_checkinteger(L, 3);

    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return 0;
    }

    lua_Integer r = luaL_checkinteger(L, 4);
    lua_Integer g = luaL_checkinteger(L, 5);
    lua_Integer b = luaL_checkinteger(L, 6);
    lua_Integer a = luaL_checkinteger(L, 7);

    lua_Integer index = (y * width + x) * 4;
    image[index] = r < 0 ? 0 : (r > 255 ? 255 : r);
    image[index + 1] = g < 0 ? 0 : (g > 255 ? 255 : g);
    image[index + 2] = b < 0 ? 0 : (b > 255 ? 255 : b);
    image[index + 3] = a < 0 ? 0 : (a > 255 ? 255 : a);

    return 0;
}

static int png_encode_file(lua_State *L)
{
    lua_getfield(L, 1, "width");
    lua_Integer width = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "height");
    lua_Integer height = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "data");
    unsigned char *image = (unsigned char *)lua_touserdata(L, -1);
    const char *filename = luaL_checkstring(L, 2);
    unsigned int error = lodepng_encode32_file(filename, image, width, height);
    if (error)
    {
        lua_pushboolean(L, 0);
        const char *error_message = lodepng_error_text(error);
        lua_pushstring(L, error_message);
        return 2;
    }
    else
    {
        lua_pushboolean(L, 1);
        lua_pushnil(L);
        return 2;
    }
}

static int png_dispose(lua_State *L)
{
    lua_getfield(L, 1, "data");
    unsigned char *image = (unsigned char *)lua_touserdata(L, -1);
    free(image);
    return 0;
}

static int png_tostring(lua_State *L)
{
    lua_getfield(L, -1, "width");
    lua_Integer width = luaL_checkinteger(L, -1);
    lua_getfield(L, -2, "height");
    lua_Integer height = luaL_checkinteger(L, -1);

    char buffer[512];
    sprintf(buffer,
            PNG_METATABLE
            "<width = %lld, height = %lld, data length = %lld bytes>",
            width, height, width * height * 4);
    lua_pushstring(L, buffer);
    return 1;
}

static const luaL_Reg png[] = {
    {"at", png_at},
    {"set", png_set},
    {"encode_file", png_encode_file},
    {"dispose", png_dispose},
    {"__tostring", png_tostring},
    {NULL, NULL},
};

static int decode_file(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    unsigned int width, height;
    unsigned char *image;
    unsigned int error = lodepng_decode32_file(&image, &width, &height, filename);

    if (error)
    {
        lua_pushnil(L);
        const char *error_message = lodepng_error_text(error);
        lua_pushstring(L, error_message);
        return 2;
    }
    else
    {
        lua_newtable(L);
        lua_pushliteral(L, "width");
        lua_pushinteger(L, width);
        lua_settable(L, -3);
        lua_pushliteral(L, "height");
        lua_pushinteger(L, height);
        lua_settable(L, -3);
        lua_pushliteral(L, "data");
        lua_pushlightuserdata(L, image);
        lua_settable(L, -3);
        luaL_setmetatable(L, PNG_METATABLE);
        lua_pushnil(L);
        return 2;
    }
}

static const luaL_Reg lodepng[] = {
    {"decode_file", decode_file},
    {NULL, NULL},
};

LUAMOD_API int luaopen_lodepng(lua_State *L)
{
    luaL_newlib(L, lodepng);
    luaL_newmetatable(L, PNG_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, png, 0);
    lua_pop(L, 1);
    return 1;
}
