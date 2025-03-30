cmake_minimum_required(VERSION 3.10)

project(VoltaNet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CRYPTOPP_DIR "${CMAKE_SOURCE_DIR}/cryptopp8.9")
set(CRYPTOPP_SOURCES
    # Core library and utilities
    "${CRYPTOPP_DIR}/cryptlib.cpp"     # Core Crypto++ functionality
    "${CRYPTOPP_DIR}/filters.cpp"      # Filter framework (used in GCM)
    "${CRYPTOPP_DIR}/misc.cpp"         # Miscellaneous utilities
    "${CRYPTOPP_DIR}/allocate.cpp"     # Memory allocation
    "${CRYPTOPP_DIR}/queue.cpp"        # Buffered transformation queue

    # Random number generation
    "${CRYPTOPP_DIR}/osrng.cpp"        # OS-provided RNG (used for nonces, keys)

    # Hashing (SHA-256 for secureHash and ECDSA)
    "${CRYPTOPP_DIR}/sha.cpp"          # SHA-1, SHA-2 implementation
    "${CRYPTOPP_DIR}/sha_simd.cpp"     # SIMD-optimized SHA (if supported)

    # Symmetric encryption (AES-GCM)
    "${CRYPTOPP_DIR}/rijndael.cpp"     # AES core
    "${CRYPTOPP_DIR}/rijndael_simd.cpp"# AES-NI optimized
    "${CRYPTOPP_DIR}/gcm.cpp"          # GCM mode for authenticated encryption
    "${CRYPTOPP_DIR}/gcm_simd.cpp"     # GCM with CLMUL (if supported)

    # Elliptic Curve Cryptography (ECDH and ECDSA)
    "${CRYPTOPP_DIR}/eccrypto.cpp"     # ECDSA and ECDH implementation
    "${CRYPTOPP_DIR}/ecp.cpp"          # Elliptic curve over prime fields
    "${CRYPTOPP_DIR}/integer.cpp"      # Arbitrary-precision integers
    "${CRYPTOPP_DIR}/nbtheory.cpp"     # Number theory (primality, modular arithmetic)
    "${CRYPTOPP_DIR}/algebra.cpp"      # Abstract algebra (groups, fields)
    "${CRYPTOPP_DIR}/asn.cpp"          # ASN.1 parsing (for key serialization)

    # Encoding (used for public key serialization)
    "${CRYPTOPP_DIR}/basecode.cpp"     # Base encoding
    "${CRYPTOPP_DIR}/hex.cpp"          # Hex encoding/decoding
)

add_library(VoltaNet STATIC
    VoltaNet.cpp
    ${CRYPTOPP_SOURCES}
)

target_include_directories(VoltaNet
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CRYPTOPP_DIR}
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mssse3 -msse4.1 -mavx -mavx2 -mpclmul -maes -msha")
endif()

if(WIN32)
    target_link_libraries(VoltaNet PRIVATE Ws2_32)
endif()