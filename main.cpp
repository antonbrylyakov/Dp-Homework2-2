#include <iostream>
#ifdef _WIN32
#include "windows.h"
#endif

#include <map>
#include <string>
#include <iostream>

class VeryHeavyDatabase {
public:
    std::string GetData(const std::string& key) const noexcept {
        return "Very Big Data String: " + key;
    }
};

class CacheProxyDB : public VeryHeavyDatabase {
public:
    // Чтобы подчеркнуть, что прокси контролирует доступ к объекту, используем unique_ptr вместо "сырого" указателя
    explicit CacheProxyDB(std::unique_ptr<VeryHeavyDatabase> real_object) : real_db_(std::move(real_object)) {}
    std::string GetData(const std::string& key) noexcept {
        if (cache_.find(key) == cache_.end()) {
            std::cout << "Get from real object\n";
            cache_[key] = real_db_->GetData(key);
        }
        else {
            std::cout << "Get from cache\n";
        }
        return cache_.at(key);
    }

private:
    std::map<std::string, std::string> cache_;
    std::unique_ptr<VeryHeavyDatabase> real_db_;
};

class TestDB : VeryHeavyDatabase {
public:
    explicit TestDB(std::unique_ptr<VeryHeavyDatabase> real_object) : real_db_(std::move(real_object)) {}
    std::string GetData(const std::string& key) noexcept {
        return "test_data\n";
    }
private:
    std::unique_ptr<VeryHeavyDatabase> real_db_;
};

class OneShotDB : public VeryHeavyDatabase {
public:
    explicit OneShotDB(std::unique_ptr<VeryHeavyDatabase> real_object, size_t shots = 1) : 
        real_db_(std::move(real_object)), shots_(shots), shot_counts_() {}
    std::string GetData(const std::string& key) noexcept {
        
        auto currCountIt = shot_counts_.find(key);
        auto currCount = currCountIt != shot_counts_.end() ? (*currCountIt).second : 0;

        if (currCount >= shots_)
        {
            return "error";
        }
        else
        {
            shot_counts_[key] = ++currCount;
            return real_db_->GetData(key);
        }
    }
private:
    std::unique_ptr<VeryHeavyDatabase> real_db_;
    size_t shots_;
    std::map<std::string, size_t> shot_counts_;
};


int main()
{
    auto one_shot_db = OneShotDB(std::make_unique<CacheProxyDB>(std::make_unique<VeryHeavyDatabase>()), 2);
    std::cout << one_shot_db.GetData("key") << std::endl;
    std::cout << one_shot_db.GetData("key") << std::endl;
    std::cout << one_shot_db.GetData("key") << std::endl;

	return 0;
}