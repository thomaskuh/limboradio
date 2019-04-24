import {RadioRequest} from './radio-request';

export class RadioResponse extends RadioRequest {

  playing: number;
  lux: number;
  timer: number;
  tagName: string;
  tagTitle: string;
  date: string;
  time: string;

  netinfo: string;

}
