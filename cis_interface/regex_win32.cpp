#include <string>
#include <regex>
#include <cstdint>


typedef std::regex_iterator<const char *> Myiter;


/*!
  @brief Count the number of times a regular expression is matched in a string.
  @param[in] regex_text constant character pointer to string that should be
  compiled into a regex.
  @param[in] to_match constant character pointer to string that should be
  checked for matches.
  @return int Number of matches found. -1 is returned if the regex could not be
  compiled.
*/
int count_matches(const char *regex_text, const char *to_match) {
  try {
    Myiter::regex_type r(regex_text);
  } catch (const std::regex_error& rerr) {
    return -1;
  }
  int ret = 0;
  Myiter next(to_match, to_match + strlen(to_match), r);
  Myiter end;
  for (; next != end; ++next)
    ret++;
  return ret;
}

/*!
  @brief Find first match to regex.
  @param[in] regex_text constant character pointer to string that should be
  compiled into a regex.
  @param[in] to_match constant character pointer to string that should be
  checked for matches.
  @param[out] sind int index where match begins.
  @param[out] eind int index where match ends.
  @return int Number of matches found. -1 is returned if the regex could not be
  compiled.
*/
int find_match(const char *regex_text, const char *to_match,
	       int *sind, int *eind) {
  try {
    std::regex r(re);
  } catch (const std::regex_error& rerr) {
    return -1;
  }
  std::cmatch m;
  int ret = 0;
  if (regex_search(to_match, to_match + strlen(to_match), m, r)) {
    ret++;
    *sind = m.position();
    *eind = *sind + m.length();
  }
  return ret;
}


/*!
  @brief Make a replacement of regex matches, ignoring captured substrings.
  @param[in,out] buf Characer pointer to buffer that replacements should be
  made to.
  @param[in] len_buf const int length of buf.
  @param[in] re Constant character pointer to regex string.
  @param[in] rp Constant character pointer to the replacement text.
  @param[in] nreplace Constant int number of replacements to make. If 0, all
  matches are replaced.
  @return int -1 on failure if the regex could not be compiled or the buffer 
  is not big enough to contain the result. If succesful, the new length of buf
  is returned.
 */
int regex_replace_nosub(char *buf, const int len_buf,
			const char *re, const char *rp,
			const int nreplace) {
  try {
    std::regex r(re);
  } catch (const std::regex_error& rerr) {
    return -1;
  }
  int ret = 0;
  int len_rp = strlen(rp);
  int len_m, rem_s, rem_l, delta_siz;
  int cur_pos = 0;
  int cur_siz = strlen(buf);
  int creplace = 0;
  std::cmatch m;
  while (1) {
    if ((nreplace > 0) && (creplace >= nreplace)) {
      printf("regex_replace_nosub: Maximum of %d replacements reached\n",
      	     creplace);
      break;
    }
    const char *first = buf + cur_pos;
    const char *last = buf + cur_siz;
    if (not regex_search(first, last, m, r)) {
      break;
    }
    // Ensure replacement will not exceed buffer
    len_m = m.length()
    delta_siz = len_rp - len_m;
    if ((cur_siz + delta_siz + 1) > len_buf) {
      printf("regex_replace_nosub: Relacement will exceed buffer.\n");
      cur_siz = -1;
      break;
    }
    // Move trailing
    rem_l = cur_siz - (cur_pos + m.position() + len_m);
    rem_s = m.position() + len_rp;
    char *p = buf + cur_pos;
    memmove(p + rem_s, p + m.position() + len_m, rem_l + 1);
    // Copy replacement
    strncpy(p + m.position(), rp, len_rp);
    // Advance
    cur_pos += rem_s;
    cur_siz += delta_siz;
    creplace += 1;
  }
  /* printf("regex_replace_nosub() = %s\n", buf); */
  return cur_siz;
}

/*!
  @brief Extract substring references from a string.
  @param[in] buf Constant character pointer to buffer that references should be
  extracted from.
  @param[out] refs Pointer to pointer to memory where reference numbers should
  be stored. This function will reallocate it to fit the number of references
  returned. (Should be freed by calling program.)
  @return int Number of refs found. -1 indicates an error.
*/
int get_subrefs(const char *buf, int **refs) {
  // Compile
  try {
    // TODO: check this on windows regex
    std::regex r("\\$([[:digit:]])");
  } catch (const std::regex_error& rerr) {
    return -1;
  }
  // Prepare "bitmap"
  const int max_ref = 10; //99;
  int i;
  uint8_t *ref_bytes = (uint8_t*)malloc((max_ref + 1)*sizeof(uint8_t));
  for (i = 0; i <= max_ref; i++)
    ref_bytes[i] = 0;
  // Locate matches
  std::cmatch m;
  const int max_grp = 2;  // Digits in max_ref
  int igrp_len;
  char igrp[max_grp];
  int iref;
  const char *first = buf;
  const char *last = buf + strlen(buf);
  while (1) {
    if (not regex_search(first, last, m, r)) {
      break;
    }
    // Lone $ without digit
    /* printf("so = %d, eo = %d\n", m[1].rm_so, m[1].rm_eo); */
    if (m.size() == 1) {
      first += m.position() + m.length();
      continue;
    }
    // Substring
    igrp_len = m.length(1);
    if (igrp_len > max_grp) {
      printf("Number longer than %d digits unlikely.\n", max_grp);
      free(ref_bytes);
      return -1;
    }
    strncpy(igrp, first + m.position(1), igrp_len);
    igrp[igrp_len] = 0;
    // Extract ref number
    iref = atoi(igrp);
    if (iref > max_ref) {
      printf("Reference to substr %d exceeds limit (%d)\n", iref, max_ref);
      free(ref_bytes);
      return -1;
    }
    ref_bytes[iref] = 1;
    first += m.position() + m.length();
  }
  // Get unique refs
  int nref = 0;
  for (i = 0; i <= max_ref; i++) {
    if (ref_bytes[i])
      nref++;
  }
  *refs = (int*)realloc(*refs, nref*sizeof(int));
  int ir;
  for (i = 0, ir = 0; i <= max_ref; i++) {
    if (ref_bytes[i]) {
      (*refs)[ir] = i;
      ir++;
    }
  }
  free(ref_bytes);
  // printf("%d refs in %s\n", nref, buf);
  return nref;
}


/*!
  @brief Make a replacement of regex matches, allowing for captured substrings.
  @param[in,out] buf Characer pointer to buffer that replacements should be
  made to.
  @param[in] len_buf const int length of buf.
  @param[in] re Constant character pointer to regex string.
  @param[in] rp Constant character pointer to the replacement text.
  @param[in] nreplace Constant int number of replacements to make. If 0, all
  matches are replaced.
  @return int -1 on failure if the regex could not be compiled or the buffer 
  is not big enough to contain the result. If succesful, the new length of buf
  is returned.
 */
int regex_replace_sub(char *buf, const int len_buf,
		      const char *re, const char *rp,
		      const int nreplace) {
  // Compile
  try {
    std::regex r(re);
  } catch (const std::regex_error& rerr) {
    return -1;
  }
  // Loop making replacements
  char * p = buf;
  std::cmatch m;
  char rp_sub[2*len_buf];
  char re_sub[len_buf];
  char igrp[len_buf];
  int len_m, rem_s, rem_l, delta_siz, len_rp;
  int cur_pos = 0;
  int cur_siz = strlen(buf);
  int creplace = 0;
  int i, j;
  while (1) {
    if ((nreplace > 0) && (creplace >= nreplace)) {
      printf("regex_replace_nosub: Maximum of %d replacements reached\n",
	     creplace);
      break;
    }
    const char *first = buf + cur_pos;
    const char *last = buf + cur_siz;
    if (not regex_search(first, last, m, r)) {
      /* printf("regex_replace_sub: nomatch for %s in %s\n", re, p); */
      break;
    }
    // Get list of subrefs
    int *refs = NULL;
    int nref = get_subrefs(rp, &refs);
    if (nref < 0) {
      printf("Error gettings subrefs\n");
      cur_siz = -1;
      break;
    }
    // For each subref complete replacements
    strcpy(rp_sub, rp);
    for (j = 0; j < nref; j++) {
      i = refs[j];
      strcpy(igrp, first + m.position(i));
      igrp[m.length(i)] = 0; // terminate
      sprintf(re_sub, "\\$%d", i);
      ret = regex_replace_nosub(rp_sub, 2*len_buf, re_sub, igrp, 0);
      if (ret < 0) {
	printf("regex_replace_sub: Error replacing substring $%d.\n", i);
	free(refs);
	return -1;
      }
    }
    // Ensure replacement will not exceed buffer
    len_rp = ret;
    len_m = m.length()
    delta_siz = len_rp - len_m;
    if ((cur_siz + delta_siz + 1) > len_buf) {
      printf("regex_replace_sub: Relacement will exceed buffer.\n");
      cur_siz = -1;
      break;
    }
    // Move trailing
    rem_l = cur_siz - (cur_pos + m.position() + m.length());
    rem_s = m.position() + len_rp;
    memmove(first + rem_s, first + m.position() + m.length(), rem_l + 1);
    // Copy replacement
    strncpy(first + m.position(), rp_sub, len_rp);
    // Advance
    cur_pos += m.position() + len_rp;
    cur_siz += delta_siz;
    creplace += 1;
    free(refs);
  }
  return cur_siz;
}
