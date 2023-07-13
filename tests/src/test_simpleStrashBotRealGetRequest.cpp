
#include "test_simpleStrashBotRealGetRequest.h"

#ifdef REALSTRASHTEST

TEST_F(_SimpleRelGetStrashBotKartInfo, KartInfo1){
    {
        std::unique_lock lock(this->m);

        this->cli->submit(qp.first);

        this->query_started= true;
        cv.notify_one();
    }

    {
        std::unique_lock lock(this->m);
        this->cv.wait(lock, [&](){return this->nb_procExecution>0;});

        EXPECT_EQ(this->nb_procExecution, 1);
        EXPECT_EQ(this->tirp.code, 200);
        EXPECT_EQ(this->tirp.gotAddress, "strashbot.fr");
    }
}

#endif //REALSTRASHTEST

