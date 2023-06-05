/**
 * @file ruuid.h
 * @author Rick Barenthin <rick@ng-voice.com>
 * @author Rick Barenthin <dunst0@gmail.com>
 * @date 12.05.2023
 * @brief This header file is for binding to the ruuid rust library.
 */

#ifndef RUUID_H
#define RUUID_H

#include <stdlib.h>

/**
 * @brief Opaque type to represent uuid from rust in the C code.
 */
typedef struct ruuid ruuid;

/**
 * @brief Max length of a formatted UUID including '\0'
 */
#define RUUID_FORMATTING_MAX_LENGTH 46

/**
 * @brief Generate a nil UUID
 * @return a pointer to a nil UUID
 */
extern ruuid *ruuid_generate_nil();
/**
 * @brief Generate a version 4 UUID
 * @return a pointer to a version 4 UUID
 */
extern ruuid *ruuid_generate_version_4();
/**
 * @brief Generate a version 5 UUID with the sip namespace
 * @note RFC 7989 - Section 4.1. Constructing the Session Identifier
 * @param name The name to be used for the version 5 UUID
 * @return a pointer to a version 5 UUID or NULL on error
 */
extern ruuid *ruuid_generate_version_5_sip(char *const name);

/**
 * @brief Parse the given UUID string
 * @param uuid_string The UUID string to be parsed
 * @return a pointer to a UUID or NULL on error
 */
extern ruuid *ruuid_parse(char *const uuid_string);

/**
 * @brief Test if the given UUID is nil UUID
 * @param uuid The UUID to tested
 * @retval -1 error in input
 * @retval 1 is a nil UUID
 * @retval 0 is not a nil UUID
 */
extern int ruuid_is_nil(ruuid *uuid);

/**
 * @brief Copy the simple formatted UUID into the given buffer
 * @param uuid The UUID to be formatted
 * @param buffer The buffer where the formatted UUID should be copied
 * @param length The size of the buffer
 * @return the number bytes copied or -1 on error
 */
extern int ruuid_get_simple(ruuid *uuid, char *buffer, size_t length);
/**
 * @brief Copy the hyphenated formatted UUID into the given buffer
 * @param uuid The UUID to be formatted
 * @param buffer The buffer where the formatted UUID should be copied
 * @param length The size of the buffer
 * @return the number bytes copied or -1 on error
 */
extern int ruuid_get_hyphenated(ruuid *uuid, char *buffer, size_t length);
/**
 * @brief Copy the urn formatted UUID into the given buffer
 * @param uuid The UUID to be formatted
 * @param buffer The buffer where the formatted UUID should be copied
 * @param length The size of the buffer
 * @return the number bytes copied or -1 on error
 */
extern int ruuid_get_urn(ruuid *uuid, char *buffer, size_t length);
/**
 * @brief Copy the braced formatted UUID into the given buffer
 * @param uuid The UUID to be formatted
 * @param buffer The buffer where the formatted UUID should be copied
 * @param length The size of the buffer
 * @return the number bytes copied or -1 on error
 */
extern int ruuid_get_braced(ruuid *uuid, char *buffer, size_t length);

/**
 * @brief Destroys a UUID
 * @param uuid The UUID to destroy
 */
extern void ruuid_destroy(ruuid *uuid);

#endif //RUUID_H
