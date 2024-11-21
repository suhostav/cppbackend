#include "GameApp.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>

namespace app {

using namespace collision_detector;

const model::GameSession* GameApp::GetPlayerSession(const Token token) const{
    Player* player = player_tokens_.FindPlayerByToken(token);
    if(player_session_.count(player) == 0){
        return nullptr;
    }
    return player_session_.at(player);
}

JoinInfo GameApp::JoinGame(std::string_view dog_name, std::string_view map_id_str){
    model::JoinResult result = game_.JoinGame(dog_name, map_id_str);
    Player* new_player = players_.Add(result.dog, result.session);
    Token token = player_tokens_.Add(*new_player);
    sessions_.insert(result.session);
    player_session_[new_player] = result.session;
    return {new_player, result.session, token};
}

void GameApp::SetPlayerSpeed(Token token, model::GameSession* session, char dir){
    app::Player* player = FindPlayerByToken(token);
    auto s = session->GetMap()->GeSpeed();
    model::DogDir dog_dir = static_cast<model::DogDir>(dir);
    DCoord limit = session->GetMap()->GetLimit(player->GetDog()->GetPoint2D(), dog_dir);
    player->GetDog()->SetDir(dog_dir, limit);
    switch(dog_dir){
        case model::DogDir::WEST:
            player->GetDog()->SetSpeed(-s, 0);
            break;
        case model::DogDir::EAST:
            player->GetDog()->SetSpeed(s, 0);
            break;
        case model::DogDir::NORTH:
            player->GetDog()->SetSpeed(0, -s);
            break;
        case model::DogDir::SOUTH:
            player->GetDog()->SetSpeed(0, s);
            break;
    }
}

    void GameApp::Move(Milliseconds period){

        players_.Move(period);
        for(auto* session : sessions_){
            session->CheckCollisions();
            session->ClearLoots();
            session->GenerateLoots(period);
        }
        std::string msg;
        PeriodicSave(period, msg);
    }

    bool GameApp::PeriodicSave(Milliseconds delta, std::string& err_msg){
        if(save_period_ == 0ms){
            return false;
        }
        time_from_save_ += delta;
        if(time_from_save_ < save_period_){
            return false;
        }
        return Save(err_msg);
        
    }

    bool GameApp::Save(std::string& err_msg){
        try{
            std::string tmp_file_name{"game_server.tmp"};
            std::ofstream out(tmp_file_name, std::ios_base::trunc);
            boost::archive::text_oarchive oa{out};
            oa << sessions_.size();
            for(const auto& session : sessions_){
                serialization::SessionRepr sr{*session};
                oa << sr;
            }
            std::vector<serialization::TokenRepr>tokens;
            for(const auto& tk : player_tokens_.GetAllTokens()){
                tokens.push_back({tk.first, tk.second});
            }
            oa << tokens;
            out.close();
            std::filesystem::rename(tmp_file_name,save_file_);
        }catch(std::exception& ex){
            err_msg = ex.what();
            return false;
        }
        time_from_save_ = 0ms;
        return true;
        
    }

    bool GameApp::Restore(std::string& err_msg){
        try{
            if(save_file_.size() == 0 || !std::filesystem::exists(save_file_)){
                return false;
            }
            
            std::ifstream in(save_file_);
            boost::archive::text_iarchive ia{in};
            size_t num;
            ia  >> num;
            std::vector<serialization::SessionRepr>sessions_repr;
            for(size_t i = 0; i < num; ++i){
                serialization::SessionRepr sr;
                ia >> sr;
                sessions_repr.push_back(sr);
            }
            std::vector<serialization::TokenRepr> tokens;
            ia >> tokens;
            for(auto& sr : sessions_repr){
                RestoreSession(sr, tokens);
            }
        }catch(std::exception& ex){
            err_msg = ex.what();
            return false;
        }
        return true;
        
    }

    void GameApp::RestoreSession(serialization::SessionRepr& session_repr, std::vector<serialization::TokenRepr>& tokens){
        model::GameSession* session = game_.CreateSession(session_repr.map_id_);
        session->SetWidth(session_repr.dog_width_, session_repr.loot_width_, session_repr.office_width_);
        for(auto& dog_repr : session_repr.dogs_){
            model::Dog dog = dog_repr.Restore();
            model::Dog* dog_ptr = session->AddDog(dog);
            Player* new_player = players_.Add(dog_ptr, session);
            auto it =  std::find_if(tokens.begin(), tokens.end(), [&dog](const serialization::TokenRepr& tr){
                return tr.GetDogId() == dog.GetId();
            });
            if(it == tokens.end()){
                throw std::out_of_range("RestoreSession: cannot find dog token.");
            }
            Token token{it->GetToken()};
            player_tokens_.Add(new_player, token);
            player_session_[new_player] = session;
        }
    }


}   //namespace app