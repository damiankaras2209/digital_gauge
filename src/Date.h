#ifndef _DATE_H
#define _DATE_H

class Date {

        static int getWeekDay(int y, int m, int d) {
            int f = y + d + 3 * m - 1;
            m++;

            if(m < 3)
                y--;
            else
                f -= int(.4 * m + 2.3);

            f += int( y / 4 ) - int( ( y / 100 + 1 ) * 0.75 );
            f %= 7;

            return f;
        }

    public:

        static int findLastSunday(int year, int month) {
            bool isleap = false;
            if(!(year % 4)) {
                if(year % 100)
                    isleap = true;
                else if(!(year % 400))
                    isleap = true;
            }

            int days[] = { 31, isleap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            int d;

            d = days[month-1];
            while( true ) {
                int x = getWeekDay(year, month-1, d);
                if(!x)
                    break;
                d--;
            }
            return d;
        }
};

#endif
