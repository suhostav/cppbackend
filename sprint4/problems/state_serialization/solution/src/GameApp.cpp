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
        time_from_save_ = 0ms;
        return Save(err_msg);
        
    }

    bool GameApp::Save(std::string& err_msg){
        try{
            std::ofstream out(tmp_save_file_, std::ios_base::trunc);
            boost::archive::text_oarchive oa{out};
            std::vector<serialization::SessionRepr> srs;
            for(auto session : sessions_){
                // std::cout << "Saving session: " << session << std::endl;
                srs.emplace_back(*session);
            }
            // std::cout << "Start saving ...\n";
            oa << srs;
            // std::cout << "sessions serialized.\n";
            std::vector<serialization::TokenRepr>tokens;
            for(const auto& tk : player_tokens_.GetAllTokens()){
                tokens.push_back({tk.first, tk.second});
            }
            oa << tokens;
            // std::cout << "Start tokens serializing...\n";
            out.close();
            std::filesystem::rename(tmp_save_file_,save_file_);
            // std::cout << "Saved!\n";
        }catch(std::exception& ex){
            err_msg = ex.what();
            return false;
        }
        return true;
    }
    

    bool GameApp::Restore(std::string& err_msg){
        try{
            if(save_file_.size() == 0 || !std::filesystem::exists(save_file_)){
                return false;
            }
            
            std::ifstream in(save_file_);
            boost::archive::text_iarchive ia{in};
            std::vector<serialization::SessionRepr>srs;
            ia >> srs;
            std::vector<serialization::TokenRepr> tokens;
            ia >> tokens;
            for(auto& sr : srs){
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
            if(model::Dog::next_id_ <= dog.GetId()){
                model::Dog::next_id_ = dog.GetId() + 1;
            }
        }
        for(serialization::LootRepr& lr : session_repr.loots_){
            session->RestoreLoot(lr.id_, lr.type_, lr.pos_);
            if(model::Loot::next_id_ <= lr.id_){
                model::Loot::next_id_ = lr.id_ + 1;
            }
        }
        sessions_.insert(session);
    }

    std::string GameApp::CreareTmpFileName(const std::string& save_file){
        std::string name;

        return save_file + ".tmp"s;
    }


}   //namespace app