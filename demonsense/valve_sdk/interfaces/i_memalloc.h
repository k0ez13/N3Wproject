#pragma once
class i_memalloc {
public:
	auto alloc( int nSize ) -> void* {
		using Fn = void*(__thiscall*)( void*, int );
		return CallVFunction< Fn >( this, 1 )( this, nSize );
	}

	auto realloc( void* pMem, int nSize ) -> void* {
		using Fn = void*(__thiscall*)( void*, void*, int );
		return CallVFunction< Fn >( this, 3 )( this, pMem, nSize );
	}

	auto free( void* pMem ) -> void {
		using Fn = void( __thiscall* )( void*, void* );
		return CallVFunction< Fn >( this, 5 )( this, pMem );
	}
};

