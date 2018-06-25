#ifndef MICROBIT_SERIALNO_H
#define MICROBIT_SERIALNO_H

// Use a binary search only if your list of serial numbers are
// in increasing numerical order. Otherwise, comment out the
// next line

#define BINARY_SEARCH 1

#define n_serials 100

int serial_nos[n_serials] = {
  -2144515396,-2059605006,-1967710724,-1961270616,-1935320890,-1932871530,
  -1912305331,-1842266494,-1821933803,-1747871151,-1711415484,-1706766978,
  -1649511589,-1600731667,-1483803288,-1434029990,-1416355700,-1335113821,
  -1332071524,-1295408325,-1259709415,-1254307818,-1250051181,-1214631201,
  -1150491640,-1126051397,-1104301737,-1062960164,-1026508348,-1008750917,
   -990323763, -953794263, -947362999, -922388356, -901230779, -892424458,
   -879249887, -784714830, -659430450, -642321921, -635379699, -467751815,
   -436323870, -262607227, -260579386, -252737854, -208204093,   59828996,
     72657792,   90474657,   90585802,  114865398,  124889300,  139504915,
    174933281,  254822985,  305655669,  312419193,  372247341,  372710108,
    482456122,  528462152,  603432698,  672447793,  716621183,  717800380,
    797439568,  811967408,  824476841,  912966444,  935683855,  955912587,
    981294339, 1054086594, 1061309998, 1100820558, 1112661040, 1132570013,
   1172785377, 1179433693, 1200292220, 1237994045, 1245353808, 1261523739,
   1264056548, 1289006107, 1352544529, 1419619065, 1447755784, 1740630402,
   1751965607, 1763580930, 1779403761, 1884718947, 1905828305, 2018132023,
   2104238997, 2122936041, 2130538896, 2146111223
 };

// Just for the record, serial no of the master micro:bit is 498461975

#ifdef BINARY_SEARCH

  short get_id_from_serial(int ser) {
    short pos = 0;
    short max = min(n_serials, 1);

    while ((max < n_serials) && (serial_nos[max] < ser)) {
      pos = max + 1;
      max = min(n_serials, max*2+1);
    }

    while (pos<max) {
      int testpos = pos + ((max - pos) >> 1);
      if (serial_nos[testpos] < ser) pos = testpos + 1;
      else max = testpos;
    }
    return (pos < n_serials && serial_nos[pos] == ser ? pos : -1);
  }

#else

  short get_id_from_serial(int ser) {
    short i = 0;
    while (i < n_serials) {
      if (serial_nos[i] == ser) {
        return i+1;
      }
      i++;
    }
    if (i == n_serials) {
      n_serials++;
      return (n_serials - 1);
    }
    return 0;
  }

#endif

#endif