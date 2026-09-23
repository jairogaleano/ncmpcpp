/***************************************************************************
 *   Copyright (C) 2008-2021 by Andrzej Rybczak                            *
 *   andrzej@rybczak.net                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "curl_handle.h"

#include <cstdlib>

namespace
{
	size_t write_data(char *buffer, size_t size, size_t nmemb, void *data)
	{
		size_t result = size*nmemb;
		static_cast<std::string *>(data)->append(buffer, result);
		return result;
	}
}

CURLcode Curl::perform(std::string &data, const std::string &URL, const std::string &referer, bool follow_redirect, unsigned timeout)
{
	CURLcode result;
	CURL *c = curl_easy_init();
	curl_easy_setopt(c, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(c, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, timeout);
	curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1);
	// Letras.com is behind Akamai bot protection: HTTP/403 unless the request
	// looks like a real browser (User-Agent + client hints + Sec-Fetch-*).
	curl_easy_setopt(c, CURLOPT_USERAGENT,
		"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36");
	curl_easy_setopt(c, CURLOPT_ACCEPT_ENCODING, "");
	struct curl_slist *headers = nullptr;
	headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
	headers = curl_slist_append(headers, "Accept-Language: es-ES,es;q=0.9,en;q=0.8");
	headers = curl_slist_append(headers, "sec-ch-ua: \"Chromium\";v=\"126\", \"Google Chrome\";v=\"126\", \"Not.A/Brand\";v=\"24\"");
	headers = curl_slist_append(headers, "sec-ch-ua-mobile: ?0");
	headers = curl_slist_append(headers, "sec-ch-ua-platform: \"Linux\"");
	headers = curl_slist_append(headers, "Sec-Fetch-Dest: document");
	headers = curl_slist_append(headers, "Sec-Fetch-Mode: navigate");
	headers = curl_slist_append(headers, "Sec-Fetch-Site: cross-site");
	headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
	if (headers)
		curl_easy_setopt(c, CURLOPT_HTTPHEADER, headers);
	if (follow_redirect)
		curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
	if (!referer.empty())
		curl_easy_setopt(c, CURLOPT_REFERER, referer.c_str());
	result = curl_easy_perform(c);
	curl_slist_free_all(headers);
	curl_easy_cleanup(c);
	return result;
}

std::string Curl::escape(const std::string &s)
{
	char *cs = curl_easy_escape(0, s.c_str(), s.length());
	std::string result(cs);
	curl_free(cs);
	return result;
}
