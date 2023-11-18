void do_showtogs(char *from, char *to, char *rest)
{
  send_to_user( from, "Status:");
  send_to_user( from, "\x2Public seen: %i \x2", seentoggle);
  send_to_user( from, "%cAuto-opping: %i\x2",'\x2', aoptoggle);
  send_to_user( from, "%cFlood protection: %i\x2",'\x2', floodtoggle);
  send_to_user( from, "\x2Mode enforcement: %i\x2", enfmtoggle);
  send_to_user( from, "\x2Shitkicking: %i\x2", shittoggle);
  send_to_user( from, "\x2Protection: %i\x2", prottoggle);
  send_to_user( from, "\x2Server-deop: %i\x2", sdtoggle);
  send_to_user( from, "\x2Idle-Kick: %i\x2", idletoggle);
  send_to_user( from, "\x2Public commands: %i\x2", pubtoggle);
  send_to_user( from, "\x2Mass mode protection: %i\x2", masstoggle);
  send_to_user( from, "\x2Welcome messages: %i\x2", weltoggle);
  send_to_user( from, "\x2Strict opping: %i\x2", soptoggle);
  send_to_user( from, "%cCommand character: %i\x2", '\x2', cctoggle);
  send_to_user( from, "\x2Logging: %i\x2", logtoggle);
  send_to_user( from, "\x2Kill logging: %i\x2", klogtoggle);
  send_to_user( from, "\x2Max-user kicking: %i\x2", mutoggle);
}


