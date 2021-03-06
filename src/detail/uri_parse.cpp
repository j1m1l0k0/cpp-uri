// Copyright 2009-2010 Jeroen Habraken.
// Copyright 2009-2013 Dean Michael Berris, Glyn Matthews.
// Copyright 2012 Google, Inc.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/qi.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include "uri_parse.hpp"

BOOST_FUSION_ADAPT_TPL_STRUCT(
    (FwdIter), (network::detail::hierarchical_part)(FwdIter),
    (boost::optional<boost::iterator_range<FwdIter> >, user_info)(
        boost::optional<boost::iterator_range<FwdIter> >, host)(
        boost::optional<boost::iterator_range<FwdIter> >, port)(
        boost::optional<boost::iterator_range<FwdIter> >, path));

BOOST_FUSION_ADAPT_TPL_STRUCT(
    (FwdIter), (network::detail::uri_parts)(FwdIter),
    (boost::optional<boost::iterator_range<FwdIter> >, scheme)(
        network::detail::hierarchical_part<FwdIter>, hier_part)(
        boost::optional<boost::iterator_range<FwdIter> >, query)(
        boost::optional<boost::iterator_range<FwdIter> >, fragment));

namespace network {
  namespace detail {
    namespace qi = boost::spirit::qi;

    template <class String>
    struct uri_grammar
        : qi::grammar<typename String::iterator,
                      detail::uri_parts<typename String::iterator>()> {

      typedef String string_type;
      typedef typename String::iterator iterator;

      uri_grammar() : uri_grammar::base_type(start, "uri") {
        // gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
        gen_delims %= qi::char_(":/?#[]@");
        // sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /
        // ";" / "="
        sub_delims %= qi::char_("!$&'()*+,;=");
        // reserved = gen-delims / sub-delims
        reserved %= gen_delims | sub_delims;
        // unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
        unreserved %= qi::alnum | qi::char_("-._~");
        // pct-encoded = "%" HEXDIG HEXDIG
        pct_encoded %= qi::char_("%") >> qi::repeat(2)[qi::xdigit];

        // pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
        pchar %=
            qi::raw[unreserved | pct_encoded | sub_delims | qi::char_(":@")];

        // segment = *pchar
        segment %= qi::raw[*pchar];
        // segment-nz = 1*pchar
        segment_nz %= qi::raw[+pchar];
        // segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
        segment_nz_nc %=
            qi::raw[+(unreserved | pct_encoded | sub_delims | qi::char_("@"))];
        // path-abempty  = *( "/" segment )
        path_abempty %= qi::raw[*(qi::char_("/") >> segment)];
        // path-absolute = "/" [ segment-nz *( "/" segment ) ]
        path_absolute %= qi::raw
            [qi::char_("/") >> -(segment_nz >> *(qi::char_("/") >> segment))];
        // path-rootless = segment-nz *( "/" segment )
        path_rootless %= qi::raw[segment_nz >> *(qi::char_("/") >> segment)];
        // path-empty = 0<pchar>
        path_empty %= qi::raw[qi::eps];

        // scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
        scheme %= qi::raw[qi::alpha >> *(qi::alnum | qi::char_("+.-"))];

        // user_info = *( unreserved / pct-encoded / sub-delims / ":" )
        user_info %=
            qi::raw[*(unreserved | pct_encoded | sub_delims | qi::char_(":"))];

        ip_literal %= qi::lit('[') >> (ipv6address | ipvfuture) >> ']';

        ipvfuture %= qi::lit('v') >> +qi::xdigit >> '.' >>
                     +(unreserved | sub_delims | ':');

        ipv6addresses[0] %= qi::repeat(6)[h16 >> ':'] >> ls32;
        ipv6addresses[1] %= "::" >> qi::repeat(5)[h16 >> ':'] >> ls32;
        ipv6addresses[2] %= -qi::raw[h16] >> "::" >> qi::repeat(4)[h16 >> ':'] 
            >> ls32;
        ipv6addresses[3] %= -qi::raw[h16] >> "::" >> qi::repeat(3)[h16 >> ':'] 
            >> ls32;
        ipv6addresses[4] %= -qi::raw[h16] >> "::" >> qi::repeat(2)[h16 >> ':'] 
            >> ls32;
        ipv6addresses[5] %= -qi::raw[h16] >> "::" >> h16 >> ':' >> ls32;
        ipv6addresses[6] %= -qi::raw[h16] >> "::" >> ls32;
        ipv6addresses[7] %= -qi::raw[h16] >> "::" >> h16;
        ipv6addresses[8] %= -qi::raw[h16] >> "::";
        ipv6addresses[9] %= -qi::raw[qi::repeat(1)[(h16 >> ':')] >> h16] >> 
            "::" >> qi::repeat(3)[h16 >> ':'] >> ls32;
        ipv6addresses[10] %= -qi::raw[qi::repeat(1)[(h16 >> ':')] >> h16] >> 
            "::" >> qi::repeat(2)[h16 >> ':'] >> ls32;
        ipv6addresses[11] %= -qi::raw[qi::repeat(1)[(h16 >> ':')] >> h16] >> 
            "::" >> h16 >> ':' >> ls32;
        ipv6addresses[12] %= -qi::raw[qi::repeat(1)[(h16 >> ':')] >> h16] >> 
            "::" >> ls32;
        ipv6addresses[13] %= -qi::raw[qi::repeat(1)[(h16 >> ':')] >> h16] >> 
            "::" >> h16;
        ipv6addresses[14] %= -qi::raw[qi::repeat(1)[(h16 >> ':')] >> h16] >> 
            "::";
        ipv6addresses[15] %= -qi::raw[qi::repeat(2)[(h16 >> ':')] >> h16] >> 
            "::" >> qi::repeat(2)[h16 >> ':'] >> ls32;
        ipv6addresses[16] %= -qi::raw[qi::repeat(2)[(h16 >> ':')] >> h16] >> 
            "::" >> h16 >> ':' >> ls32;
        ipv6addresses[17] %= -qi::raw[qi::repeat(2)[(h16 >> ':')] >> h16] >> 
            "::" >> ls32;
        ipv6addresses[18] %= -qi::raw[qi::repeat(2)[(h16 >> ':')] >> h16] >> 
            "::" >> h16;
        ipv6addresses[19] %= -qi::raw[qi::repeat(2)[(h16 >> ':')] >> h16] >> 
            "::";
        ipv6addresses[20] %= -qi::raw[qi::repeat(3)[(h16 >> ':')] >> h16] >> 
            "::" >> h16 >> ':' >> ls32;
        ipv6addresses[21] %= -qi::raw[qi::repeat(3)[(h16 >> ':')] >> h16] >> 
            "::" >> ls32;
        ipv6addresses[22] %= -qi::raw[qi::repeat(3)[(h16 >> ':')] >> h16] >> 
            "::" >> h16;
        ipv6addresses[23] %= -qi::raw[qi::repeat(3)[(h16 >> ':')] >> h16] >> 
            "::";
        ipv6addresses[24] %= -qi::raw[qi::repeat(4)[(h16 >> ':')] >> h16] >> 
            "::" >> ls32;
        ipv6addresses[25] %= -qi::raw[qi::repeat(4)[(h16 >> ':')] >> h16] >> 
            "::" >> h16;
        ipv6addresses[26] %= -qi::raw[qi::repeat(4)[(h16 >> ':')] >> h16] >> 
            "::";
        ipv6addresses[27] %= -qi::raw[qi::repeat(5)[(h16 >> ':')] >> h16] >> 
            "::" >> h16;
        ipv6addresses[28] %= -qi::raw[qi::repeat(5)[(h16 >> ':')] >> h16] >> 
            "::";
        ipv6addresses[29] %= -qi::raw[qi::repeat(6)[(h16 >> ':')] >> h16] >> 
            "::";

        ipv6address %= qi::raw
            [ipv6addresses[0] |
            ipv6addresses[1] |
            ipv6addresses[2] |
            ipv6addresses[3] |
            ipv6addresses[4] |
            ipv6addresses[5] |
            ipv6addresses[6] |
            ipv6addresses[7] |
            ipv6addresses[8] |
            ipv6addresses[9] |
            ipv6addresses[10] |
            ipv6addresses[11] |
            ipv6addresses[12] |
            ipv6addresses[13] |
            ipv6addresses[14] |
            ipv6addresses[15] |
            ipv6addresses[16] |
            ipv6addresses[17] |
            ipv6addresses[18] |
            ipv6addresses[19] |
            ipv6addresses[20] |
            ipv6addresses[21] |
            ipv6addresses[22] |
            ipv6addresses[23] |
            ipv6addresses[24] |
            ipv6addresses[25] |
            ipv6addresses[26] |
            ipv6addresses[27] |
            ipv6addresses[28] |
            ipv6addresses[29]];

        // ls32 = ( h16 ":" h16 ) / IPv4address
        ls32 %= (h16 >> ':' >> h16) | ipv4address;

        // h16 = 1*4HEXDIG
        h16 %= qi::repeat(1, 4)[qi::xdigit];

        // dec-octet = DIGIT / %x31-39 DIGIT / "1" 2DIGIT / "2" %x30-34 DIGIT /
        // "25" %x30-35
        dec_octet %= !(qi::lit('0') >> qi::digit) >>
                     qi::raw[qi::uint_parser<boost::uint8_t, 10, 1, 3>()];

        // IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
        ipv4address %=
            qi::raw[dec_octet >> qi::repeat(3)[qi::lit('.') >> dec_octet]];

        // reg-name = *( unreserved / pct-encoded / sub-delims )
        reg_name %= qi::raw[*(unreserved | pct_encoded | sub_delims)];

        // host = IP-literal / IPv4address / reg-name
        // This should be priority ordering, but Spirit is greedy
        // and will fail if 'host' starts with 'ipv4address'. We make
        // sure there isn't another '.' after last IPv4 octet.
        host %= qi::raw[ip_literal | (ipv4address >> !(&qi::lit('.'))) | reg_name];

        port %= qi::raw[qi::ushort_ | qi::eps];

        // query = *( pchar / "/" / "?" )
        query %= qi::raw[*(pchar | qi::char_("/?"))];

        // fragment = *( pchar / "/" / "?" )
        fragment %= qi::raw[*(pchar | qi::char_("/?"))];

        // hier-part = "//" authority path-abempty / path-absolute /
        // path-rootless / path-empty
        // authority = [ userinfo "@" ] host [ ":" port ]
        hier_part %=
            ((("//" >> user_info >> '@') | "//") >> host >> -(':' >> port) >>
             path_abempty) |
            (qi::attr(boost::optional<boost::iterator_range<iterator> >()) >>
             qi::attr(boost::optional<boost::iterator_range<iterator> >()) >>
             qi::attr(boost::optional<boost::iterator_range<iterator> >()) >>
             (path_absolute | path_rootless | path_empty));

        start %=
            ((scheme >> ':') >> hier_part >> -('?' >> query) >>
             -('#' >> fragment));
      }

      qi::rule<iterator, typename boost::iterator_range<iterator>::value_type()>
          gen_delims, sub_delims, reserved, unreserved;
      qi::rule<iterator, string_type()> pct_encoded, pchar;

      qi::rule<iterator, string_type()> segment, segment_nz, segment_nz_nc;
      qi::rule<iterator, boost::iterator_range<iterator>()> path_abempty,
          path_absolute, path_rootless, path_empty;

      qi::rule<iterator, string_type()> dec_octet, ipv4address, reg_name,
          ipv6address, ipvfuture, ip_literal;

      qi::rule<iterator, string_type()> ipv6addresses[30];

      qi::rule<iterator, string_type()> h16, ls32;

      qi::rule<iterator, boost::iterator_range<iterator>()> host, port;

      qi::rule<iterator, boost::iterator_range<iterator>()> scheme, user_info,
          query, fragment;

      qi::rule<iterator, detail::hierarchical_part<iterator>()> hier_part;

      // actual uri parser
      qi::rule<iterator, detail::uri_parts<iterator>()> start;
    };

    bool parse(uri::string_type::iterator first,
               uri::string_type::iterator last,
               uri_parts<uri::string_type::iterator> &parts) {
      namespace qi = boost::spirit::qi;
      static uri_grammar<uri::string_type> grammar;
      bool is_valid = qi::parse(first, last, grammar, parts);
      return is_valid && (first == last);
    }
  }  // namespace detail
}  // namespace network
