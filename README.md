# AFIT-CSCE489-PROJ2

Big todo right now: I'm running into the issue where the consumers will want to buy a yoda after the producer is done producing them, then will block on the wait() call indefinitely. I need some way to prevent that, though I'm not sure if it's better implemented in the consumer routine or in the semaphore object. 